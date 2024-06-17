# C++ Shellcode Execution Template
The C++ Shellcode Execution Template is used by the C2 server to recompile a payload that will be executed by the SodaMaster implant for each command execution task. The compiled binary will be converted to shellcode and executed by the SodaMaster implant, which will then pass the output of the execution back to the C2 server.

The template will find, load, and lock the resource located in the `bin` folder (should be replaced with the payload you want to write to disk) and then write it to disk at the desired path (`PAYLOAD_PATH`) if the payload name is supplied. It will then create a **cmd.exe** process with the desired args (`ARGS`) and a named pipe to write the output of the process to if CLI arguments are supplied.

## Build Instructions
When using the C++ template in conjunction with the SodaMaster implant and C2 server, the SodaMaster TCP handler will pass in the required parameters and recompile the template with the desired payload once the execution task has been assigned to the implant. There is no need to manually build the template in this case. For instructions on how to task the SodaMaster implant, see [here](..//README.md#tasking).

The template is configured to cross compile Windows on a Linux machine. Instructions for manually building the template are below.

**From Powershell/Terminal:**

1. Install the necessary [dependencies](#dependencies) on your machine.

2. Replace the bytes of the `payload.exe` executable in the `template\bin` folder with your desired payload bytes. Keep the name of the resource as `payload.exe`.

3. Navigate to the `.../sodamaster/template` directory in a terminal and run the following commands:
    ```
    cmake -S . -B build -DPAYLOAD_PATH="path\to\write\payload" -DPAYLOAD_NAME="payload name" -DARGS="any command line arguments"
    cmake --build build --clean-first
    ```

### Dependencies

**Linux:**
- CMake
    ```
    sudo apt install cmake
    ```
- MinGW
    ```
    sudo apt install mingw-w64
    ```

## Execution

**Linux:**

Since the binary is compiled for Windows, it will not run on a Linux machine without a compatibility layer. See [here](https://en.wikipedia.org/wiki/Compatibility_layer) for more information.

**Windows:**

Open Powershell and navigate to the `...\sodamaster\template\bin` directory. Either run the `test_template.exe` executable or the `template.dll` DLL with `rundll32.exe` and exported function "Run" to execute the template binary.

## Testing

Compile the exe/dll with the build commands above. After the build is complete, run the `test.exe` executable concurrently with `test_template.exe` or `template.dll`. Monitor your terminal to see the test output.

The `...\template\testing` code is pulled from the SodaMaster implant, and can be found [here](../../../../SodaMaster/SodaMaster-DLL/NamedPipe.cpp).

## Cleanup
- Delete the payload from disk on the victim machine (should be located at `PAYLOAD_PATH` with `PAYLOAD_NAME`).

## Resources
1. https://securelist.com/apt10-sophisticated-multi-layered-loader-ecipekac-discovered-in-a41apt-campaign/101519/
2. http://jsac.jpcert.or.jp/archive/2022/pdf/JSAC2022_9_yanagishita-tamada-nakatsuru-ishimaru_en.pdf
3. https://media.kasperskydaily.com/wp-content/uploads/sites/86/2021/02/25140359/greatidea_A41_v1.0.pdf
4. https://www.trendmicro.com/ja_jp/research/21/l/Sigloader-by-Earth-Tengshe.html