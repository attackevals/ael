# SSM Hybrid Activation Role

This role registers both Linux and Windows endpoints to AWS Systems Manager using Hybrid Activations.

## Variables

- `ssm_activation_code`: The activation code generated in AWS Systems Manager.
- `ssm_activation_id`: The activation ID generated in AWS Systems Manager.
- `ssm_region`: The AWS region where Systems Manager is configured.
- `ssm_agent_download_url` (optional): Custom download URL for the SSM Agent. Only used if SSM Agent not found in default path (both Windows/Linux).

## Example Playbook

```yaml
- hosts: all
  become: yes
  roles:
    - role: ssm_hybrid_activation
      vars:
        ssm_activation_code: "YOUR_ACTIVATION_CODE"
        ssm_activation_id: "YOUR_ACTIVATION_ID"
        ssm_region: "us-east-1"
```
