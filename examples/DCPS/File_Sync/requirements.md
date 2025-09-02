File_Sync: Software Requirements Specification (SRS)
Version 1.0

1. Introduction
   1.1 Purpose
   This document provides a detailed specification for the File_Sync application. File_Sync is a continuous file synchronization program designed to securely and safely synchronize directories of text files between two or more computers over a local area network (LAN). The system will be built using C++ and will leverage OpenDDS for its underlying data distribution and communication framework.
   1.2 Project Scope
   The project's scope is to develop a robust, headless application that monitors a source directory for changes (file creation, modification, deletion) and propagates these changes to all other connected instances of the application. Each instance will also subscribe to changes from its peers and update its own destination directory accordingly. The primary focus is on data integrity, security, and automated, user-friendly operation.
   1.3 Core Goals
   The application's design and implementation will be guided by the following goals, listed in order of importance:
   Safe From Data Loss: Protecting the user's data is paramount.
   Secure Against Attackers: Data must be protected from eavesdropping and unauthorized modification.
   Easy to Use: The application should be approachable and understandable.
   Automatic: User interaction should be required only when absolutely necessary.
   Universally Available: The application should run on common computer systems.
   For Individuals: The primary user is the individual, though enterprise use is possible.
2. Overall Description
   2.1 System Architecture
   File_Sync operates as a peer-to-peer distributed system. There is no central server. Each instance of the application is a peer (a DDS Participant) in a shared OpenDDS domain. Each peer acts as both a publisher of its own local file changes and a subscriber to the changes from all other peers.
   OpenDDS handles the complexities of peer discovery, data marshaling, and reliable transport over the network. Security, including authentication and encryption, is managed by the OpenDDS Security Specification, with UnityAuth serving as the specific implementation.
   2.2 OpenDDS Data Model
   To facilitate efficient and reliable data transfer, the application will use two primary DDS topics. This model separates control information from bulk data transfer, a best practice for this type of application.
   2.2.1 FileMetadata Topic
   This topic is used to announce file state changes. It is a lightweight control message that alerts subscribers to new files, modifications, or deletions.
   // In FileSync.idl
   module FileSync {

#pragma DCPS_DATA_TYPE "FileSync::FileMetadata"
#pragma DCPS_DATA_KEY "FileSync::FileMetadata file_id"
struct FileMetadata {
string file_id; // Unique identifier (e.g., relative path from source dir). Key field.
long long mod_time; // Last modification timestamp (e.g., seconds since epoch).
string file_hash; // Cryptographic hash (SHA-256) of the full file content.
boolean is_deleted; // Flag to indicate if the file has been deleted.
};

};

2.2.2 FileChunk Topic
This topic is used to transfer the actual file content in manageable pieces. This approach avoids exceeding DDS message size limits, ensures reliable transfer of file content, and allows for future optimizations.
// In FileSync.idl
module FileSync {

#pragma DCPS_DATA_TYPE "FileSync::FileChunk"
#pragma DCPS_DATA_KEY "FileSync::FileChunk file_id, sequence_number"
struct FileChunk {
string file_id; // Corresponds to FileMetadata.file_id. Part of the key.
long sequence_number; // The ordered number of this chunk. Part of the key.
long total_chunks; // The total number of chunks for this file.
sequence<octet> data; // The raw byte data for this chunk.
};

};

3. Functional Requirements (FR)
   FR1: Directory Monitoring
   FR1.1: The application must recursively monitor a user-specified source directory for file events.
   FR1.2: It must detect the creation of new files within the source directory tree.
   FR1.3: It must detect the modification of existing files.
   FR1.4: It must detect the deletion of existing files.
   FR2: Data Publication via OpenDDS
   FR2.1: Upon detecting a new or modified file, the application must first publish a FileMetadata sample with is_deleted set to false.
   FR2.2: Following the metadata publication, the application must read the file, break it into chunks, and publish a series of FileChunk samples.
   FR2.3: Upon detecting a file deletion, the application must publish a FileMetadata sample for that file with is_deleted set to true.
   FR3: Data Subscription and File Reconstruction
   FR3.1: The application must subscribe to the FileMetadata and FileChunk topics to receive data from peers.
   FR3.2: Upon receiving a FileMetadata sample where is_deleted is false, the application must prepare to receive chunks for that file_id.
   FR3.3: The application must collect all FileChunk samples for a given file and reassemble them in the correct order based on sequence_number.
   FR3.4: Once all chunks are received, the application must compute the hash of the reassembled data and verify it against the file_hash from the metadata. If the hashes match, the file must be written to the destination directory.
   FR3.5: Upon receiving a FileMetadata sample where is_deleted is true, the application must delete the corresponding file from its destination directory.
   FR4: Configuration
   FR4.1: The application must allow the user to specify the source (monitoring) directory.
   FR4.2: The application must allow the user to specify the destination (receiving) directory.
   FR4.3: The application must provide a mechanism for configuring OpenDDS settings, including Domain ID and security document paths for UnityAuth.
   FR5: Conflict Resolution
   FR5.1: If the application receives a file update for a file that has also been modified locally (i.e., the local file's hash does not match the last known synchronized state), it must not overwrite the local file.
   FR5.2: The incoming version must be saved as a new file in the destination directory.
   FR5.3: The new file's name must be clearly marked as a conflicted copy, appending a suffix such as (conflicted copy from <peer_hostname> YYYY-MM-DD).txt.
4. Non-Functional Requirements (NFR)
   NFR1: Safety & Data Integrity (Goal 1)
   NFR1.1: File content integrity must be verified using a SHA-256 hash. A file will only be written to the destination directory if the hash of the received data matches the hash provided in the metadata.
   NFR1.2: File write operations must be atomic to prevent data corruption. The application must write reassembled file content to a temporary file and perform an atomic move/rename operation upon successful hash validation.
   NFR2: Security (Goal 2)
   NFR2.1: All OpenDDS communication (discovery and user data) must be secured using the OMG DDS-Security specification.
   NFR2.2: The application must be configured to use UnityAuth for authentication, access control, and transport-level encryption (TLS/DTLS) over the LAN.
   NFR3: Usability & Automation (Goals 3 & 4)
   NFR3.1: The application must run as a non-interactive background process (daemon/service) after initial configuration.
   NFR3.2: Peer discovery and connection management must be handled automatically by OpenDDS.
   NFR4: Performance (Goal 7)
   NFR4.1: The application must be optimized to handle thousands of text files where typical file sizes are less than 200KB.
   NFR4.2: The data transfer mechanism (FileChunk size) and DDS Quality of Service (QoS) policies should be tuned for high-throughput, low-latency LAN environments. A RELIABLE reliability QoS will be used for all data writers and readers.
   NFR5: Portability & Availability (Goal 5)
   NFR5.1: The application must be developed in standards-compliant C++ (C++17 or newer).
   NFR5.2: The build system must be CMake to support compilation on major operating systems (Linux, Windows, macOS).
   NFR5.3: Filesystem operations should use a cross-platform library (e.g., std::filesystem or Boost.Filesystem) to ensure portability.
   NFR6: Target Audience (Goal 6)
   NFR6.1: The configuration and operation, while powerful, should be as simple as possible to empower individual users to manage their own file synchronization.
