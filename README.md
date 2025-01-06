# FTP Server and Client Implementation

This project implements an FTP Server and Client based on the [RFC-959](https://www.rfc-editor.org/rfc/rfc959) standard. It provides functionality for file transfer between systems, supporting both active and passive modes.

## Features

- **File Transfer:**
  - Transfers files byte by byte from one location to another.
  - Tested extensively using PNG files to ensure accuracy and reliability.

- **Modes Supported:**
  - Active mode.
  - Passive mode.

- **Security Measures:**
  - Implemented safeguards against File Traversal attacks.
  - Input strings are thoroughly validated to prevent potential exploits.

## Technical Details

- **Programming Language:**
  - Written in C++.

- **Target Platform:**
  - Designed specifically for Windows machines, leveraging OS-specific functions.

---

