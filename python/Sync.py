import os
import shutil
import stat
import time
import sys

def sync_folders(source, replica, log_file):
    try:
        with open(log_file, "a") as log:
            for entry in os.listdir(source):
                source_path = os.path.join(source, entry)
                replica_path = os.path.join(replica, entry)

                try:
                    file_stat = os.stat(source_path)

                    if stat.S_ISDIR(file_stat.st_mode):
                        #Check if directory exists in replica
                        if not os.path.exists(replica_path):
                            os.makedirs(replica_path)
                            print("Created directory {}".format(replica_path))
                            log.write("Created directory {}\n".format(replica_path))

                        #Recursively synchronize subdirectories
                        sync_folders(source_path, replica_path, log_file)
                    elif stat.S_ISREG(file_stat.st_mode):
                        #Check if the entry exists in both source and replica
                        if os.path.exists(replica_path):
                            with open(source_path, "rb") as src, open(replica_path, "rb") as rep:
                                is_different = any(a != b for a, b in zip(src.read(), rep.read()))

                            if is_different:
                                #Remove the file from replica
                                os.remove(replica_path)
                                print("Deleted {} from {}".format(entry, replica_path))
                                log.write("Deleted {} from {}\n".format(entry, replica_path))
                                shutil.copy2(source_path, replica_path)
                                print("Copied {} to {}".format(source_path, replica_path))
                                log.write("Copied {} to {}\n".format(source_path, replica_path))
                        else:
                            shutil.copy2(source_path, replica_path)
                            print("Copied {} to {}".format(source_path, replica_path))
                            log.write("Copied {} to {}\n".format(source_path, replica_path))
                except Exception as e:
                    print("Error processing {}: {}".format(source_path, str(e)))
                    log.write("Error processing {}: {}\n".format(source_path, str(e)))

    except Exception as e:
        print("Error opening log file: {}".format(str(e)))
        return 1

def delete_subfolders_and_files(path, log):
    try:
        for entry in os.listdir(path):
            replica_path = os.path.join(path, entry)

            try:
                if os.path.isdir(replica_path):
                    delete_subfolders_and_files(replica_path, log)
                    os.rmdir(replica_path)
                    print("Deleted {}".format(replica_path))
                    log.write("Deleted {}\n".format(replica_path))
                elif os.path.isfile(replica_path):
                    os.remove(replica_path)
                    print("Deleted file from {}".format(replica_path))
                    log.write("Deleted file from {}\n".format(replica_path))

            except Exception as e:
                print("Error deleting {}: {}".format(replica_path, str(e)))
    except Exception as e:
        print("Error opening directory {}: {}".format(path, str(e)))
        return 1

def check_replica(source, replica, log):
    try:
        for entry in os.listdir(replica):
            source_path = os.path.join(source, entry)
            replica_path = os.path.join(replica, entry)

            try:
                file_stat = os.stat(replica_path)

                if stat.S_ISDIR(file_stat.st_mode) and not os.path.exists(source_path):
                    #If the entry exists in replica but not in source
                    delete_subfolders_and_files(replica_path, log)
                    os.rmdir(replica_path)
                    print("Deleted {}".format(replica_path))
                    log.write("Deleted {}\n".format(replica_path))
                elif stat.S_ISREG(file_stat.st_mode) and not os.path.exists(source_path):
                    #If the entry exists in source but not in replica
                    os.remove(replica_path)
                    print("Deleted {} from {}".format(entry, replica_path))
                    log.write("Deleted {} from {}\n".format(entry, replica_path))

            except Exception as e:
                print("Error processing {}: {}".format(replica_path, str(e)))
                log.write("Error processing {}: {}\n".format(replica_path, str(e)))

    except Exception as e:
        print("Error opening replica directory: {}".format(str(e)))
        return 1

def termination(source_path, replica_path, log):
    if source_path:
        del source_path
    if replica_path:
        del replica_path
    if log:
        log.close()

def main():
    if len(sys.argv) != 5:
        print("ERROR: correct -> ./programName source replica interval logFile")
        return 1

    source = sys.argv[1]
    replica = sys.argv[2]
    interval = int(sys.argv[3])
    log_file = sys.argv[4]

    while True:
        if sync_folders(source, replica, log_file) == 1:
            print("Error with synchronization")
            return 1
        check_replica(source, replica, log_file)
        time.sleep(interval)

if __name__ == "__main__":
    main()
