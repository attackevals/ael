#!/usr/bin/env python3

import os
import re
from typing import Any, Dict, List, Optional, Tuple

import yaml

Config = Dict[str, str]
TaskSummary = Tuple[str, Optional[str]]


def load_config(config_path: str = "ansible-docs-config.yml") -> Config:
    with open(config_path, "r") as f:
        config: Config = yaml.safe_load(f)
    required = {"ansible_root", "playbook_dir", "reusable_task_dir", "output_file"}
    missing = required - config.keys()
    if missing:
        raise ValueError(f"Missing config keys: {missing}")
    return config


def extract_required_vars(task_file: str) -> List[str]:
    vars_ = []
    try:
        with open(task_file, "r") as f:
            lines = f.readlines()
        for i, line in enumerate(lines):
            if line.strip().lower().startswith("# required variables:"):
                for j in range(i + 1, len(lines)):
                    match = re.match(r"#\s*-\s*(\S+)", lines[j])
                    if match:
                        vars_.append(match.group(1))
                    elif lines[j].strip() and not lines[j].strip().startswith("#"):
                        break
                break
    except Exception:
        pass
    return vars_


def get_task_summaries(path: str) -> List[TaskSummary]:
    summaries = []
    try:
        with open(path, "r") as f:
            data = yaml.safe_load(f)
        if isinstance(data, list):
            for task in data:
                if isinstance(task, dict):
                    name = task.get("name", "Unnamed task")
                    fqcn = next((k for k in task if "." in k), None)
                    summaries.append((name, fqcn))
    except Exception:
        pass
    return summaries


def fqcn_link(fqcn: Optional[str]) -> str:
    if not fqcn:
        return "builtin/unknown"
    parts = fqcn.split(".")
    if len(parts) != 3:
        return fqcn
    base_url = "https://docs.ansible.com/ansible/latest/collections"
    return f"[{fqcn}]({base_url}/{parts[0]}/{parts[1]}/{parts[2]}_module.html)"


def summarize_roles(ansible_root: str) -> List[Dict[str, Any]]:
    roles_dir = os.path.join(ansible_root, "roles")
    summaries = []

    if not os.path.isdir(roles_dir):
        return summaries

    for role in sorted(os.listdir(roles_dir)):
        role_path = os.path.join(roles_dir, role)
        if not os.path.isdir(role_path):
            continue
        summary = {"name": role, "vars": [], "tasks": []}
        for var_src in ["defaults", "vars"]:
            var_file = os.path.join(role_path, var_src, "main.yml")
            if os.path.exists(var_file):
                try:
                    with open(var_file, "r") as f:
                        data = yaml.safe_load(f)
                        if isinstance(data, dict):
                            summary["vars"].extend(data.keys())
                except Exception:
                    pass
        task_file = os.path.join(role_path, "tasks", "main.yml")
        summary["tasks"] = get_task_summaries(task_file)
        summaries.append(summary)
    return summaries


def summarize_playbooks(ansible_root: str, playbook_dir: str) -> List[Dict[str, Any]]:
    playbooks = []
    full_path = os.path.join(ansible_root, playbook_dir)
    if not os.path.isdir(full_path):
        return playbooks
    for fname in sorted(os.listdir(full_path)):
        if fname.endswith(".yml"):
            fpath = os.path.join(full_path, fname)
            playbooks.append({"name": fname, "tasks": get_task_summaries(fpath)})
    return playbooks


def summarize_reusable_tasks(ansible_root: str, task_dir: str) -> List[Dict[str, Any]]:
    reusable = []
    full_path = os.path.join(ansible_root, task_dir)
    if not os.path.isdir(full_path):
        return reusable
    for fname in sorted(os.listdir(full_path)):
        if fname.endswith(".yml"):
            fpath = os.path.join(full_path, fname)
            reusable.append(
                {
                    "file": fname,
                    "vars": extract_required_vars(fpath),
                    "tasks": get_task_summaries(fpath),
                },
            )
    return reusable


def generate_markdown(
    roles: List[Dict[str, Any]],
    playbooks: List[Dict[str, Any]],
    reusable_tasks: List[Dict[str, Any]],
) -> str:
    lines = ["# Ansible Documentation", ""]

    lines.append("## Table of Contents")
    lines.append("- [Roles](#roles)")
    lines.append("- [Playbooks](#playbooks)")
    lines.append("- [Reusable Tasks](#reusable-tasks)")
    lines.append("\n---\n")

    lines.append("## Roles")
    for r in roles:
        lines.append(f"\n### `{r['name']}`\n")
        lines.append("| Task Name | Module | Variables |")
        lines.append("|-----------|--------|-----------|")
        if not r["tasks"]:
            lines.append("| _No tasks_ | | |")
        for name, fqcn in r["tasks"]:
            var_str = ", ".join(r["vars"]) if r["vars"] else ""
            lines.append(f"| {name} | {fqcn_link(fqcn)} | {var_str} |")

    lines.append("\n## Playbooks")
    for pb in playbooks:
        lines.append(f"\n### `{pb['name']}`\n")
        lines.append("| Task Name | Module |")
        lines.append("|-----------|--------|")
        if not pb["tasks"]:
            lines.append("| _No tasks_ | |")
        for name, fqcn in pb["tasks"]:
            lines.append(f"| {name} | {fqcn_link(fqcn)} |")

    lines.append("\n## Reusable Tasks")
    for t in reusable_tasks:
        lines.append(f"\n### `{t['file']}`")
        if t["vars"]:
            lines.append(
                f"**Required Variables:** `{', '.join(t['vars'])}`\n",
            )
        lines.append("| Task Name | Module |")
        lines.append("|-----------|--------|")
        if not t["tasks"]:
            lines.append("| _No tasks_ | |")
        for name, fqcn in t["tasks"]:
            lines.append(f"| {name} | {fqcn_link(fqcn)} |")

    return "\n".join(lines)


def main() -> None:
    config = load_config()
    roles = summarize_roles(config["ansible_root"])
    playbooks = summarize_playbooks(config["ansible_root"], config["playbook_dir"])
    reusable = summarize_reusable_tasks(
        config["ansible_root"],
        config["reusable_task_dir"],
    )
    md = generate_markdown(roles, playbooks, reusable)
    with open(config["output_file"], "w") as f:
        f.write(md)
    print(f"📘 Generated {config['output_file']}")


if __name__ == "__main__":
    main()
