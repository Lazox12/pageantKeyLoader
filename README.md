
# pageantKeyLoader

pageantKeyLoader is a utility designed to automate the loading of SSH keys into Pageant, streamlining the process of managing SSH key authentication on Windows systems.

## Features

- **Automated Key Loading**: Automatically loads specified SSH keys into Pageant upon execution.
- **Configuration Management**: Utilizes a configuration file to specify which keys to load, allowing for easy updates and management.
- **Command-Line Interface**: Offers a simple command-line interface for integration into scripts and automation workflows.

## Prerequisites

- **Windows Operating System**: This utility is intended for use on Windows platforms.
- **Pageant**: Ensure that Pageant is installed and accessible in your system's PATH. Pageant is a part of the PuTTY suite and can be downloaded from the [official website](https://www.chiark.greenend.org.uk/~sgtatham/putty/latest.html).

## Installation
**Method 1 (primary for not nerdy people :D ):**
1. download the precompiled pageantKeyLoader.exe from releases.
2. move it to its own directory for example I use ```C:/software/pageantKeyLoader```

**Method 2 :** 
1. **Clone the Repository**:
   ```bash
   git clone https://github.com/Lazox12/pageantKeyLoader.git
   ```
2. **Clone the SQLitecpp dependency**
   ````bash
   git submodule init
   git submodule update
   ````

3.**Build the Application**:
   ```bash
   cd pageantKeyLoader
   mkdir build
   cd build
   cmake ..
   make
   ```

   *Note*: Ensure that you have CMake and a compatible C++ compiler installed on your system.


## Usage

1. **Prepare Your SSH Keys**:
   - Place your private SSH key files in a secure directory.
   - Ensure that the keys are in the PuTTY Private Key (*.ppk) format. If your keys are in OpenSSH format, you can convert them using PuTTYgen.

2. **Configure the Key Loader**:
   - Execute the pageantKeyLoader.exe
   - than press ```a``` to add key and then provide a file name
   - or press ```f``` to add all keys in a given folder
   
    
## Automating Key Loading at Startup

To have your SSH keys loaded into Pageant automatically at system startup:

1. **Create a Shortcut**:
   - Navigate to the directory containing `pageantKeyLoader.exe`.
   - Right-click on `pageantKeyLoader.exe` and select "Create shortcut".


2. **Modify shortcut**:
   - Right-click on the shortcut and click properties.
   - Under the Target tag add ```-a``` to the end.


3**Move the Shortcut to the Startup Folder**:
   - Press `Win + R`, type `shell:startup`, and press Enter to open the Startup folder.
   - Move the previously created shortcut into this folder.

This setup ensures that `pageantKeyLoader` runs at each system startup, loading your specified SSH keys into Pageant automatically.

## Contributing

Contributions are welcome! Please fork the repository and submit a pull request with your enhancements or bug fixes.

## License

This project is licensed under the MIT License. See the `LICENSE` file for more details.
This project is using the sqlitecpp as dependency so check their licence too
