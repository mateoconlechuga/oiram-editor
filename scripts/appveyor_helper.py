# appveyor helper script for downloading and installing question
# copied from CEmu (https://raw.githubusercontent.com/CE-Programming/CEmu/master/gui/qt/deploy-scripts/appveyor_helper.py)
# copyright (C) Albert "alberthdev" Huang

import os
import sys
import hashlib
import re
import subprocess
import time
import glob
import zipfile
import shutil
import requests
import json

# Timeout socket handling
import socket
import threading
import errno

from util import *

try:
    import zlib
    compression = zipfile.ZIP_DEFLATED
except:
    compression = zipfile.ZIP_STORED

modes = { zipfile.ZIP_DEFLATED: 'deflated',
          zipfile.ZIP_STORED:   'stored',
          }

try:
    # Python 3
    from urllib.request import urlopen, Request
    from urllib.error import HTTPError, URLError
    from urllib.parse import urlparse
    from http.client import HTTPException
except ImportError:
    # Python 2
    from urllib2 import urlopen, Request, HTTPError, URLError
    from urlparse import urlparse
    from httplib import HTTPException

BINTRAY_SNAPSHOT_SERVER_PATH = "https://oss.jfrog.org/artifactory/oss-snapshot-local"
BINTRAY_RELEASE_SERVER_PATH = "https://oss.jfrog.org/artifactory/oss-release-local"
MAX_ATTEMPTS = 5
SHA256_STRICT = False

# Solution from Anppa @ StackOverflow
# http://stackoverflow.com/a/18468750/1094484
def timeout_http_body_read_to_file(response, fh, timeout = 60):
    def murha(resp):
        os.close(resp.fileno())
        resp.close()

    # set a timer to yank the carpet underneath the blocking read() by closing the os file descriptor
    t = threading.Timer(timeout, murha, (response,))

    t.start()
    fh.write(response.read())
    t.cancel()

def truncate_url(url):
    if len(url) > 70:
        truncated_url = url[:34] + ".." + url[len(url) - 34:]
    else:
        truncated_url = url

    return truncated_url

def check_file(path):
    try:
        test_fh = open(path)
        test_fh.close()
        return True
    except IOError:
        return False

# Note: suppress_errors will only work on HTTP errors
# Other errors will be forcefully displayed
def check_url(url, suppress_errors = True):
    check_attempts = 0
    found = False

    while check_attempts <= MAX_ATTEMPTS:
        # If we aren't on our first download attempt, wait a bit.
        if check_attempts > 0:
            print("         !! Download attempt failed, waiting 10s before retry...")
            print("            (attempt %i/%i)" % (check_attempts + 1, MAX_ATTEMPTS))

            # Wait...
            time.sleep(10)

        # Open the url
        try:
            f = urlopen(url)

            # Everything good!
            found = True
            break

        # Error handling...
        except HTTPError:
            if not suppress_errors:
                _, e, _ = sys.exc_info()
                print("         !! HTTP Error: %i (%s)" % (e.code, url))
                print("         !! Server said:")
                err = e.read().decode("utf-8")
                err = "         !! " + "\n         !! ".join(err.split("\n")).strip()
                print(err)

            found = False
            break
        except URLError:
            _, e, _ = sys.exc_info()
            print("         !! URL Error: %s (%s)" % (e.reason, url))

            found = False
            break
        except HTTPException:
            _, e, _ = sys.exc_info()
            print("         !! HTTP Exception: %s (%s)" % (str(e), url))
        except socket.error:
            _, e, _ = sys.exc_info()
            if e.errno == errno.EBADF:
                print("         !! Timeout reached: %s (%s)" % (str(e), url))
            else:
                print("         !! Socket Exception: %s (%s)" % (str(e), url))

        # Increment attempts
        check_attempts += 1

    if check_attempts > MAX_ATTEMPTS:
        print("         !! ERROR: URL check failed, assuming not found!")

    return found

def dlfile(url, dest = None):
    dl_attempts = 0
    dest = dest or os.path.basename(url)

    while dl_attempts <= MAX_ATTEMPTS:
        # If we aren't on our first download attempt, wait a bit.
        if dl_attempts > 0:
            print("         !! Download attempt failed, waiting 10s before retry...")
            print("            (attempt %i/%i)" % (dl_attempts + 1, MAX_ATTEMPTS))

            # Wait...
            time.sleep(10)

        # Open the url
        try:
            f = urlopen(url)
            print("         -> Downloading:")
            print("            %s" % truncate_url(url))

            # Open our local file for writing
            with open(dest, "wb") as local_file:
                timeout_http_body_read_to_file(f, local_file, timeout = 300)
                #local_file.write(f.read())

            # Everything good!
            break

        # Error handling...
        except HTTPError:
            _, e, _ = sys.exc_info()
            print("         !! HTTP Error: %i (%s)" % (e.code, url))
            print("         !! Server said:")
            err = e.read().decode("utf-8")
            err = "         !! " + "\n         !! ".join(err.split("\n")).strip()
            print(err)
        except URLError:
            _, e, _ = sys.exc_info()
            print("         !! URL Error: %s (%s)" % (e.reason, url))
        except HTTPException:
            _, e, _ = sys.exc_info()
            print("         !! HTTP Exception: %s (%s)" % (str(e), url))
        except socket.error:
            _, e, _ = sys.exc_info()
            if e.errno == errno.EBADF:
                print("         !! Timeout reached: %s (%s)" % (str(e), url))
            else:
                print("         !! Socket Exception: %s (%s)" % (str(e), url))

        # Increment attempts
        dl_attempts += 1

    if dl_attempts > MAX_ATTEMPTS:
        print("         !! ERROR: Download failed, exiting!")
        sys.exit(1)

def generate_file_md5(filename, blocksize=2**20):
    m = hashlib.md5()
    with open( filename , "rb" ) as f:
        while True:
            buf = f.read(blocksize)
            if not buf:
                break
            m.update( buf )
    return m.hexdigest()

def generate_file_sha1(filename, blocksize=2**20):
    m = hashlib.sha1()
    with open( filename , "rb" ) as f:
        while True:
            buf = f.read(blocksize)
            if not buf:
                break
            m.update( buf )
    return m.hexdigest()

def generate_file_sha256(filename, blocksize=2**20):
    m = hashlib.sha256()
    with open( filename , "rb" ) as f:
        while True:
            buf = f.read(blocksize)
            if not buf:
                break
            m.update( buf )
    return m.hexdigest()

def output_md5(filename):
    md5_result = "%s  %s" % (filename, generate_file_md5(filename))
    print(md5_result)
    return md5_result

def output_sha1(filename):
    sha1_result = "%s  %s" % (filename, generate_file_sha1(filename))
    print(sha1_result)
    return sha1_result

def output_sha256(filename):
    sha256_result = "%s  %s" % (filename, generate_file_sha256(filename))
    print(sha256_result)
    return sha256_result

# True if valid, False otherwise
# Generalized validation function
#   filename    - file to check
#   chksum_file - checksum file to verify against
#   hash_name   - name of hash function used
#   hash_regex  - regex to validate the hash format
#   hash_func   - function to create hash from file
def validate_gen(filename, chksum_file, hash_name, hash_regex, hash_func):
    print("      -> Validating file with %s: %s" % (hash_name, filename))
    try:
        hash_fh = open(chksum_file)
        correct_hash = hash_fh.read().strip()
        hash_fh.close()
    except IOError:
        print("      !! ERROR: Could not open checksum file '%s' for opening!" % chksum_file)
        print("      !!        Exact error follows...")
        raise

    # Ensure hash is a valid checksum
    hash_match = re.match(hash_regex, correct_hash)

    if not hash_match:
        print("      !! ERROR: Invalid %s checksum!" % hash_name)
        print("      !!        Extracted %s (invalid): %s" % (hash_name, correct_hash))
        sys.exit(1)

    # One more thing - check to make sure the file exists!
    try:
        test_fh = open(filename, "rb")
        test_fh.close()
    except IOError:
        print("      !! ERROR: Can't check %s checksum - could not open file!" % hash_name)
        print("      !!        File: %s" % filename)
        print("      !! Traceback follows:")
        traceback.print_exc()
        return False

    # Alright, let's compute the checksum!
    cur_hash = hash_func(filename)

    # Check the checksum...
    if cur_hash != correct_hash:
        print("      !! ERROR: %s checksums do not match!" % hash_name)
        print("      !!        File: %s" % filename)
        print("      !!        Current %s: %s" % (hash_name, cur_hash))
        print("      !!        Correct %s: %s" % (hash_name, correct_hash))
        return False

    # Otherwise, everything is good!
    return True

def validate(filename):
    valid_md5 = validate_gen(filename, filename + ".md5", "MD5", r'^[0-9a-f]{32}$', generate_file_md5)

    if valid_md5:
        valid_sha1 = validate_gen(filename, filename + ".sha1", "SHA1", r'^[0-9a-f]{40}$', generate_file_sha1)

        if valid_sha1:
            # Special case: SHA256.
            # Check its existence before attempting to validate.
            if check_file(filename + ".sha256") or SHA256_STRICT:
                valid_sha256 = validate_gen(filename, filename + ".sha256", "SHA256", r'^[0-9a-f]{64}$', generate_file_sha256)

                return valid_sha256
            else:
                print("      !! **********************************************************")
                print("      !! WARNING: SHA256 checksum was not found for file:")
                print("         %s" % filename)
                print("         SHA256 checksum is strongly suggested for file integrity")
                print("         checking due to the weakness of other hashing algorithms.")
                print("         Continuing for now.")
                print("      !! **********************************************************")
                return True
        else:
            return False # alternatively, valid_sha1
    else:
        return False # alternatively, valid_md5

def dl_and_validate(url):
    validation_attempts = 0
    local_fn = os.path.basename(url)

    print("   -> Downloading + validating:")
    print("      %s" % truncate_url(url))

    # Download checksums...
    print("      -> Downloading checksums for file: %s" % (local_fn))
    dlfile(url + ".md5")
    dlfile(url + ".sha1")

    # SHA256 support was recently added, so do some careful checking
    # here.
    if check_url(url + ".sha256"):
        dlfile(url + ".sha256")
    else:
        # https://oss.jfrog.org/artifactory/oss-snapshot-local/org/github/alberthdev/cemu/appveyor-qt/Qt5.12.0_Rel_Static_Win32_DevDeploy.7z
        # https://oss.jfrog.org/api/storage/oss-snapshot-local/org/github/alberthdev/cemu/appveyor-qt/Qt5.12.0_Rel_Static_Win32_DevDeploy.7z
        url_parsed = urlparse(url)
        if url_parsed.netloc == "oss.jfrog.org" and url_parsed.path.startswith("/artifactory"):
            file_info_json_url = url.replace("://oss.jfrog.org/artifactory/", "://oss.jfrog.org/api/storage/")

            if check_url(file_info_json_url):
                dlfile(file_info_json_url, local_fn + ".info.json")

                file_info_json_fh = open(local_fn + ".info.json")
                file_info_json = json.loads(file_info_json_fh.read())
                file_info_json_fh.close()

                if "checksums" in file_info_json and "sha256" in file_info_json["checksums"]:
                    print("      -- Found SHA256 checksum from file JSON info.")
                    print("         SHA256: %s" % file_info_json["checksums"]["sha256"])
                    sha256_fh = open(local_fn + ".sha256", "w")
                    sha256_fh.write("%s" % file_info_json["checksums"]["sha256"])
                    sha256_fh.close()
                else:
                    print("      !! Could not find SHA256 checksum in JSON info.")
            else:
                print("      !! JSON info does not seem to work or exist:")
                print("         %s" % file_info_json_url)
                print("         Will not be able to locate SHA256 checksum.")
        else:
            print("      !! Could not detect a OSS JFrog URL. Will not be able")
            print("         to find SHA256 checksum.")

    while validation_attempts < MAX_ATTEMPTS:
        # If we aren't on our first download attempt, wait a bit.
        if validation_attempts > 0:
            print("      !! Download + validation attempt failed, waiting 10s before retry...")
            print("         (attempt %i/%i)" % (validation_attempts + 1, MAX_ATTEMPTS))
            # Wait...
            time.sleep(10)

        print("      -> Downloading file: %s" % (local_fn))

        # Download file...
        dlfile(url)

        # ...and attempt to validate it!
        if validate(local_fn):
            break

        # Validation failed... looping back around.
        # Increment validation attempt counter
        validation_attempts += 1

    if validation_attempts > MAX_ATTEMPTS:
        print("      !! ERROR: Download and validation failed, exiting!")
        sys.exit(1)

    print("      -> Downloaded + validated successfully:")
    print("         %s" % truncate_url(url))

def extract(filename):
    print("   -> Extracting file: %s" % filename)
    if not silent_exec(["7z", "x", "-y", "-oC:\\", filename]):
        print("   !! ERROR: Failed to extract file: " % filename)
        print("   !!        See above output for details.")
        sys.exit(1)

def install_deps():
    print(" * Attempting to download dependencies...")
    dl_and_validate('https://oss.jfrog.org/artifactory/oss-snapshot-local/org/github/alberthdev/cemu/appveyor-qt/Qt5.12.0_Rel_Static_Win32_DevDeploy.7z.001')
    dl_and_validate('https://oss.jfrog.org/artifactory/oss-snapshot-local/org/github/alberthdev/cemu/appveyor-qt/Qt5.12.0_Rel_Static_Win32_DevDeploy.7z.002')
    dl_and_validate('https://oss.jfrog.org/artifactory/oss-snapshot-local/org/github/alberthdev/cemu/appveyor-qt/Qt5.12.0_Rel_Static_Win64_DevDeploy.7z.001')
    dl_and_validate('https://oss.jfrog.org/artifactory/oss-snapshot-local/org/github/alberthdev/cemu/appveyor-qt/Qt5.12.0_Rel_Static_Win64_DevDeploy.7z.002')

    print(" * Attempting to install dependencies...")
    extract('Qt5.12.0_Rel_Static_Win32_DevDeploy.7z.001')
    extract('Qt5.12.0_Rel_Static_Win64_DevDeploy.7z.001')

    print(" * Successfully installed build dependencies!")

def usage(msg = None):
    if msg:
        print(msg)

    print("Usage: %s install" % sys.argv[0])
    sys.exit(1)

if __name__ == "__main__":
    if len(sys.argv) != 2:
        usage()
        sys.exit(1)

    elif sys.argv[1] == "install":
        install_deps()
    else:
        usage("ERROR: Invalid command!")
