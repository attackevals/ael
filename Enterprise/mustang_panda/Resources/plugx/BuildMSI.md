# Setup Build

## Setup Visual Studio Environment

- Open Visual Studio
- install Microsoft Visual Studio Installer Projects extension
  - Go to Extensions -> Manage extensions in Visual Studio. Go to the Online section, look for the extension, and download it. The installation will be scheduled after you close Visual Studio. When you reopen it, the extension will be ready to use.

## Create MSI Project

- Open Visual Studio
- Go to File -> New -> Project
  - Set to "All Languages", "All platforms", "All project types" and then search for "Setup Project"
  - Select "Setup Project" (NOT Setup Wizard)
  - Set an appropriate name and location (e.g. within overall implant cmake folder if you plan on using the MSI to package implant components during the cmake build)
  - Select "create new solution"
  - Select "Place solution and project in the same directory"
  - Click "Create"

## Set Installation Directory

- If Solution Explorer is not visible, go to View -> Solution Explorer
- Open the File System Editor by right-clicking the MSI project (which is one level down from the top-level of the Solution) and then selecting View -> File System.
  - By default, you should see 3 folders.
- Right click on "Application Folder" and then select Add -> File. Within the file explorer, browse to the file you want the MSI to install, and repeat for any other files needed.
  - Note - if the MSI installer is going to be built as part of an overall cmake workflow and will be packaging other components built by the same workfow, you will need to reference the file paths at build, not at install, since the MSI installer will be built before cmake installs any files.
- Within the Properties window for "Application Folder", change "DefaultLocation" if needed.

## Set Custom Action

To set a custom action to execute one of the dropped files after installation, do the following:

- Open the Custom Actions Editor by right-clicking the MSI project under the solution and then selecting View -> Custom Actions.
- Right click on Install -> Add Custom Action
- Double click on Application Folder
- Choose the executable to run
- In the Custom Actions Editor window, right click the executable under Install and select Properties Window
  - Change the name as necessary
  - Adjust Arguments as necessary
  - Set "Run64Bit" to true

## Set Deployment Project Properties

To adjust the MSI properties, do the following:

- Right-click one of the files under the MSI project and then select "Properties". Then click on the MSI project to load the MSI properties in the property window.
- Change properties as required
  - Set "Author", "Description", "Manufacturer", "Product Name", and "Title" appropriately based on CTI
  - Keep "InstallAllUsers" set to False unless otherwise required.
  - Set Target Platform to x64
- Right-click on the MSI project and click "Properties" to open up the Properties page window. For all Configurations, set the output filename accordingly and make sure that "Package files" is set to "In setup file." Apply changes.

## Set Privilege Condition

To make sure the MSI installer only runs when elevated, do the following:

- Open the Launch Conditions Editor by right-clicking the MSI project under the solution and then selecting View -> Launch Conditions.
- Right click on Launch Conditions -> Add Launch Condition
  - Set the Name and Description as desired
  - Set the Condition to: `Privileged=1`

## Build

To build via command line, run:

```powershell
devenv /build Release project.sln
```

## References

- <https://www.advancedinstaller.com/create-msi-installer-with-visual-studio.html>
- <https://learn.microsoft.com/en-us/windows/win32/msi/privileged>
- <https://learn.microsoft.com/en-us/windows/win32/msi/adminuser>
- <https://www.c-sharpcorner.com/UploadFile/87ad51/3/>
- <https://stackoverflow.com/a/311837>
- <https://learn.microsoft.com/en-us/windows/win32/msi/msiuserealadmindetection>
- <https://github.com/RicoSuter/ProjectDependencyBrowser/blob/master/src/MyToolkit.Build/ProjectTypeGuidMapper.cs>
- <https://learn.microsoft.com/en-us/windows/win32/msi/property-reference?redirectedfrom=MSDN>
