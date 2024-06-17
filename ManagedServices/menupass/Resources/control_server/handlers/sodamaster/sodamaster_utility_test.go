package sodamaster

import (
    "testing"
)

var (
    testHexString  = "54686973206973206120746573742068657820737472696E672E"
    testImplantHex = []string{"00", "02", "31", "32", "48", "65", "6C", "6C", "6F", "20", "57", "6F", "72", "6C", "64", "21"}
    testID         = "w"
    testData       = "Hello World!"
    testDataLen    = int32(12)
)

func TestFormatTask(t *testing.T) {
    //want: id: "w" -> 77, len of len: 2 -> 02, len: 12 -> 0102, data: "Hello World!" -> to hex 48656c6c6f20576f726c6421
    want := "7702010248656c6c6f20576f726c6421"
    result := formatTask(testID, testDataLen, testData)
    if want != result {
        t.Errorf("Expected: %s, got %s", want, result)
    }
}

func TestFormatShellcodeTask(t *testing.T) {
    //want: id: "w" -> 77, len of len: 2 -> 02, b64 len: 16 -> 0106, data: []byte("Hello World!") -> to b64 -> to hex 53475673624738675632397962475168
    want := "7702010653475673624738675632397962475168"
    result := formatShellcodeTask(testID, []byte(testData))
    if want != result {
        t.Errorf("Expected: %s, got %s", want, result)
    }
}

func TestDecodeHex(t *testing.T) {
    wantL2, wantDataLen, wantData := 2, 12, "Hello World!"
    resL2, resDataLen, resData, err := decodeHex(testImplantHex)
    if err != nil {
        t.Error(err.Error())
    }
    if int64(wantL2) != resL2 {
        t.Errorf("Expected %d; got: %d", wantL2, resL2)
    }
    if int32(wantDataLen) != resDataLen {
        t.Errorf("Expected %d; got: %d", wantDataLen, resDataLen)
    }
    if wantData != resData {
        t.Errorf("Expected %s; got: %s", wantData, resData)
    }
}

// Uncomment for testing `prepExecutionTask` on a machine with cmake and mingw installed.

// func TestPrepExecutionTask(t *testing.T) {
//     // set current working directory to main repo directory to access ./files
//     cwd, _ := os.Getwd()
//     os.Chdir("../../")
//     defer os.Chdir(cwd) // restore cwd at end of test

//     //Call prepExecutionTask with tasking struct so we can test all the values
//     resultShellcode, err := prepExecutionTask(&exampleExecuteTaskStruct)
//     if err != nil {
//         t.Errorf(err.Error())
//     }

//     //Check that bin/payload.exe has been overwritten / bytes match
//     resultPayload, err := os.ReadFile("./handlers/sodamaster/template/bin/payload.exe")
//     if err != nil {
//         t.Errorf(err.Error())
//     }
//     if bytes.Compare(resultPayload, exampleExecuteTaskStruct.payloadData) != 0 {
//         t.Errorf("Expected %v, got %v", exampleExecuteTaskStruct.payloadData, resultPayload)
//     }

//     //Check that the dll was built and check for example payload bytes
//     resultDLL, err := os.ReadFile(dllPath)
//     if err != nil {
//         t.Errorf("Expected to find template DLL.")
//     }
//     if !bytes.Contains(resultDLL, exampleExecuteTaskStruct.payloadData) {
//         t.Errorf("DLLs do not match.")
//     }

//     //Check that the sRDI repo exists
//     if _, err = os.Open(sRDIpath); os.IsNotExist(err) {
//         t.Errorf("Expected to find sRDI repo.")
//     }

//     //Check that bin file exists and isn't empty
//     if _, err = os.Open(templatePath + "/bin/libtemplate.bin"); os.IsNotExist(err) {
//         t.Errorf("Expected to find bin file.")
//     }
//     if len(resultShellcode) == 0 {
//         t.Errorf("Shellcode not valid.")
//     }
// }
