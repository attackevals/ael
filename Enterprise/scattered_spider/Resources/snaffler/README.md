# Snaffler

Snaffler is an open source secret scanning tool used to discover credentials from Windows & Active Directory environments.

Repository adopted at [commit 429ad8bd4ab33f415937ec1f30550cea00dfb6cc](https://github.com/SnaffCon/Snaffler/tree/429ad8bd4ab33f415937ec1f30550cea00dfb6cc).

* [Original Repository](https://github.com/SnaffCon/Snaffler/tree/master)
* [Original License](https://github.com/SnaffCon/Snaffler/blob/master/LICENSE)
* [Original README](https://github.com/SnaffCon/Snaffler/blob/master/README.md)

## Features

* enumerates Windows computers from Active Directory for readable file shares
* enumerates the files in the discovered file shares for hidden credentials

## Usage

* Populate your testing environment with dummy and credential files
  * [Random file generator](https://github.com/attackevals/ael/tree/main/Enterprise/wizard_spider/Resources/setup/file_generator)

* To access the help menu for Snaffler:

  ```cmd
  .\Snaffler --help
  ```

  Output:

  ```cmd
   .::::::.:::.    :::.  :::.    .-:::::'.-:::::':::    .,:::::: :::::::..
  ;;;`    ``;;;;,  `;;;  ;;`;;   ;;;'''' ;;;'''' ;;;    ;;;;'''' ;;;;``;;;;
  '[==/[[[[, [[[[[. '[[ ,[[ '[[, [[[,,== [[[,,== [[[     [[cccc   [[[,/[[['
    '''    $ $$$ 'Y$c$$c$$$cc$$$c`$$$'`` `$$$'`` $$'     $$""   $$$$$$c
   88b    dP 888    Y88 888   888,888     888   o88oo,.__888oo,__ 888b '88bo,
    'YMmMY'  MMM     YM YMM   ''` 'MM,    'MM,  ''''YUMMM''''YUMMMMMMM   'W'
                           by l0ss and Sh3r4 - github.com/SnaffCon/Snaffler



  Usage:
          -e, --timeout[optional]... Interval between status updates (in minutes) also acts as a timeout for AD data to be gathered via LDAP. Turn this knob up if you aren't getting any computers from AD when you run Snaffler through a proxy or other slow link. Default = 5

          -z, --config[optional]... Path to a .toml config file. Run with 'generate' to puke a sample config file into the working directory.

          -o, --outfile[optional]... Path for output file. You probably want this if you're not using -s.

          -h, --help[optional]... Displays this help.

          -s, --stdout[optional]... Enables outputting results to stdout as soon as they're found. You probably want this if you're not using -o.

          -m, --snaffle[optional]... Enables and assigns an output dir for Snaffler to automatically snaffle a copy of any found files.

          -l, --snafflesize[optional]... Maximum size of file to snaffle, in bytes. Defaults to 10MB.

          -i, --dirtarget[optional]... Disables computer and share discovery, requires a path to a directory in which to perform file discovery.

          -b, --interest[optional]... Interest level to report (0-3)

          -d, --domain[optional]... Domain to search for computers to search for shares on to search for files in. Easy.

          -v, --verbosity[optional]... Controls verbosity level, options are Trace (most verbose), Debug (less verbose), Info (less verbose still, default), and Data (results only). e.g '-v debug'

          -c, --domaincontroller[optional]... Domain controller to query for a list of domain computers.

          -r, --maxgrepsize[optional]... The maximum size file (in bytes) to search inside for interesting strings. Defaults to 500k.

          -j, --grepcontext[optional]... How many bytes of context either side of found strings in files to show, e.g. -j 200

          -u, --domainusers[optional]... Makes Snaffler grab a list of interesting-looking accounts from the domain and uses them in searches.

          -y, --tsv[optional]... Makes Snaffler output as tsv.

          -f, --dfs[optional]... Limits Snaffler to finding file shares via DFS, for "OPSEC" reasons.

          -a, --sharesonly[optional]... Stops after finding shares, doesn't walk their filesystems.

          -x, --maxthreads[optional]... How many threads to be snaffling with. Any less than 4 and you're gonna have a bad time.

          -n, --comptarget[optional]... List of computers in a file(e.g C:        argets.txt), a single Computer (or comma separated list) to target.

          -p, --rulespath[optional]... Path to a directory full of toml-formatted rules. Snaffler will load all of these in place of the default ruleset.

          -t, --logtype[optional]... Type of log you would like to output. Currently supported options are plain and JSON. Defaults to plain.

          -k, --exclusions[optional]... Path to a file containing a list of computers to exclude from scanning.


  I snaffled 'til the snafflin was done.
  ```

* To run Snaffler against a specific drive and output to STDOUT as soon as results are discovered:

  ```cmd
  .\Snaffler -s -i D:\
  ```

  Output:

  ```cmd
  PS C:\Users\drstrange\Downloads> .\Snaffler.exe -s -i D:\
   .::::::.:::.    :::.  :::.    .-:::::'.-:::::':::    .,:::::: :::::::..
  ;;;`    ``;;;;,  `;;;  ;;`;;   ;;;'''' ;;;'''' ;;;    ;;;;'''' ;;;;``;;;;
  '[==/[[[[, [[[[[. '[[ ,[[ '[[, [[[,,== [[[,,== [[[     [[cccc   [[[,/[[['
    '''    $ $$$ 'Y$c$$c$$$cc$$$c`$$$'`` `$$$'`` $$'     $$""   $$$$$$c
   88b    dP 888    Y88 888   888,888     888   o88oo,.__888oo,__ 888b '88bo,
    'YMmMY'  MMM     YM YMM   ''` 'MM,    'MM,  ''''YUMMM''''YUMMMMMMM   'W'
                           by l0ss and Sh3r4 - github.com/SnaffCon/Snaffler


  [SABER\drstrange@bravo] 2024-10-14 18:12:16Z [Info] Parsing args...
  [SABER\drstrange@bravo] 2024-10-14 18:12:16Z [Info] Parsed args successfully.
  [SABER\drstrange@bravo] 2024-10-14 18:12:16Z [Info] Creating a TreeWalker task for D:\
  [SABER\drstrange@bravo] 2024-10-14 18:12:16Z [Info] Created all TreeWalker tasks.
  [SABER\drstrange@bravo] 2024-10-14 18:12:17Z [File] {Red}<KeepAwsKeysInCode|R|(\s|\'|\"|\^|=)(A3T[A-Z0-9]|AKIA|AGPA|AROA|AIPA|ANPA|ANVA|ASIA)[A-Z2-7]{12,16}(\s|\'|\"|$)|199B|2024-10-14 16:30:49Z>(D:\Documents\new_key.txt) [default]\naws_access_key_id = AKIAQYLPMN5HHHFPZAM2\naws_secret_access_key = 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\nusername = drstrange\npassword = LJKJfsdj-HH3095nn\noutput = json\nregion = us-east-2\n
  [SABER\drstrange@bravo] 2024-10-14 18:12:17Z [File] {Red}<KeepPassOrKeyInCode|R|passw?o?r?d\s*=\s*[\'\"][^\'\"]....|215B|2024-10-14 18:12:04Z>(D:\Desktop\creds.config) <?xml version="1.0" encoding="utf-8"?>\n<configuration>\n<startup><supportedRuntime version="v4.0" sku=".NETFramework,Version=v4.5"/></startup></configuration>\n<user="snaffle"/><password="KJKJKDFRES-JKjkjkj-3343"/>
  [SABER\drstrange@bravo] 2024-10-14 18:12:17Z [File] {Black}<KeepSSHKeysByFileName|R|^id_rsa$|2.5kB|2024-10-03 14:01:04Z>(D:\Desktop\keys\id_rsa) id_rsa
  [SABER\drstrange@bravo] 2024-10-14 18:12:17Z [File] {Red}<KeepPassOrKeyInCode|R|passw?o?r?d\s*=\s*[\'\"][^\'\"]....|215B|2024-10-14 18:12:04Z>(D:\Desktop\creds.config) <?xml version="1.0" encoding="utf-8"?>\n<configuration>\n<startup><supportedRuntime version="v4.0" sku=".NETFramework,Version=v4.5"/></startup></configuration>\n<user="snaffle"/><password="KJKJKDFRES-JKjkjkj-3343"/>
  [SABER\drstrange@bravo] 2024-10-14 18:12:17Z [File] {Red}<KeepInlinePrivateKey|R|-----BEGIN( RSA| OPENSSH| DSA| EC| PGP)? PRIVATE KEY( BLOCK)?-----|2.5kB|2024-10-03 14:01:04Z>(D:\Desktop\keys\id_rsa) -----BEGIN\ OPENSSH\ PRIVATE\ KEY-----\nb3BlbnNzaC1rZXktdjEAAAAABG5vbmUAAAAEbm9uZQAAAAAAAAABAAABlwAAAAdzc2gtcn\nNhAAAAAwEAAQAAAYEArVsWSBsmSIymxVih73E9MCB3MhULgX/9WqPA/m8IcF9EpoKmvvX/\nmPz8sHS1bh0\+1EqKP07/CM1vvApCc/T/CZeKE/p\+EfI8RUEj4yiFHt/ErAZWrxYrThEr2P\n8cmA/iQIAgASKnZqkMsLtUjAYn1qIbmNO/PczlkFwF5ZSw534mU\+gTGlCArmOtbG4vMy3L\nhF0rAHbU/Z8lieETE8REin7RadxU\+Y2K2lOxrBZSCCE\+cGh\+Zus1zvBRQGMzbtK66a6E6i\n5plT0/nqT
  [SABER\drstrange@bravo] 2024-10-14 18:17:16Z [Info] Status Update:
  ShareFinder Tasks Completed: 0
  ShareFinder Tasks Remaining: 0
  ShareFinder Tasks Running: 0
  TreeWalker Tasks Completed: 1
  TreeWalker Tasks Remaining: 3
  TreeWalker Tasks Running: 3
  FileScanner Tasks Completed: 84
  FileScanner Tasks Remaining: 20
  FileScanner Tasks Running: 20
  77.1MB RAM in use.

  ShareScanner queue finished, rebalancing workload.
  Insufficient FileScanner queue size, rebalancing workload.
  Max ShareFinder Threads: 0
  Max TreeWalker Threads: 21
  Max FileScanner Threads: 39
  Been Snafflin' for 00:05:00.0174243 and we ain't done yet...

  [SABER\drstrange@bravo] 2024-10-14 18:17:16Z [Info] Status Update:
  ShareFinder Tasks Completed: 0
  ShareFinder Tasks Remaining: 0
  ShareFinder Tasks Running: 0
  TreeWalker Tasks Completed: 4
  TreeWalker Tasks Remaining: 0
  TreeWalker Tasks Running: 0
  FileScanner Tasks Completed: 104
  FileScanner Tasks Remaining: 0
  FileScanner Tasks Running: 0
  77.1MB RAM in use.

  Insufficient FileScanner queue size, rebalancing workload.
  Max ShareFinder Threads: 0
  Max TreeWalker Threads: 22
  Max FileScanner Threads: 38
  Been Snafflin' for 00:05:00.0330815 and we ain't done yet...

  [SABER\drstrange@bravo] 2024-10-14 18:17:16Z [Info] Finished at 10/14/2024 6:17:16 PM
  [SABER\drstrange@bravo] 2024-10-14 18:17:16Z [Info] Snafflin' took 00:05:00.0330815
  Snaffler out.
  I snaffled 'til the snafflin was done.
  ```

* To run Snaffler against a specific directory and output to log file:

  ```cmd
  .\Snaffler -o .\snaffler.log -i D:\
  ```

* To run Snaffler against a specific host:

  ```cmd
  .\Snaffler -s -n <host name>
  ```

## Build Instructions

#### Visual Studio

* Open the `...\Resources\snaffler` directory in Visual Studio

* Load the `Snaffler.sln` solution file

* Build -> Build all

#### Command Line

* From the `...\Resources\snaffler` directory

  ```cmd
  MSBuild.exe Snaffler.sln /p:Configuration="Release"
  ```

## CTI Reporting

1. <https://www.crowdstrike.com/blog/analysis-of-intrusion-campaign-targeting-telecom-and-bpo-companies/>
1. <https://unit42.paloaltonetworks.com/muddled-libra/>
