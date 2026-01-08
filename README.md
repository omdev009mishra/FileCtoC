# PC-to-PC File Transfer Program

A C++ program for direct file transfer between two computers using **Ethernet** or **USB** cables.

**NEW: Windows GUI Application Available!** 🎉

## Available Versions

### 1. **GUI Application (Recommended)** 
Easy-to-use Windows application with graphical interface - no command line needed!

### 2. **Command Line Tools**
Traditional console-based file transfer utilities

## Supported Connection Types

✅ **Ethernet Cable (RJ45)** - Standard network cable  
✅ **USB-C to USB-C** - Direct connection with Mobile Hotspot  
✅ **USB-C/A to Ethernet Adapters** - USB adapters + Ethernet cable  
✅ **USB Direct Link Cable** - Special USB cable with built-in chip

## Requirements

- Windows 10/11
- One of the supported cables (see above)
- C++ compiler (MinGW-w64 or Visual Studio)

---

# GUI Application Guide

## Building the GUI App

Simply run the build script:
```bash
build_app.bat
```

This will:
1. Automatically create an application icon
2. Compile the GUI application with resources
3. Create `FileTransferApp.exe`

## Using the GUI App

1. **Launch the application:** Double-click `FileTransferApp.exe`

2. **Scan your network:**
   - Click "Scan Network" to see all available connections
   - The app will show Ethernet, USB, and WiFi connections with their IP addresses
   - Double-click any connection to auto-fill its IP address

3. **Choose your mode:**
   - **Receive Files (Server):** Select this on the PC that will receive files
   - **Send Files (Client):** Select this on the PC that will send files

4. **Transfer files:**
   
   **To receive files:**
   - Select "Receive Files (Server)" mode
   - Click "Start Transfer"
   - Wait for the other PC to connect
   
   **To send files:**
   - Select "Send Files (Client)" mode
   - Click "Browse..." to select a file
   - Enter the target PC's IP address (or double-click from the network list)
   - Click "Start Transfer"

5. **Monitor progress:**
   - The progress bar shows transfer status
   - Status messages appear in the bottom text area
   - Received files are saved to your Desktop by default

### GUI Features

- ✅ Network scanner to find available connections
- ✅ Visual progress bar
- ✅ Real-time status updates
- ✅ Simple file browser
- ✅ One-click IP address selection
- ✅ Stop transfer button
- ✅ No command line knowledge required!

---

# Command Line Tools Guide

## Compilation

### Using MinGW-w64:
```bash
g++ file_transfer.cpp -o file_transfer.exe -lws2_32 -liphlpapi
g++ usb_network_setup.cpp -o usb_helper.exe -lws2_32 -liphlpapi
```

### Using Visual Studio:
```bash
cl file_transfer.cpp /EHsc ws2_32.lib iphlpapi.lib
cl usb_network_setup.cpp /EHsc ws2_32.lib iphlpapi.lib
```

---

## Quick Start Guide

### Step 1: Choose Your Connection Method

#### Option A: Ethernet Cable (Easiest)
1. Connect both PCs with an Ethernet/RJ45 cable
2. Skip to Step 3

#### Option B: USB-C to USB-C Direct
1. Connect both PCs with USB-C cable
2. **On PC 1 (Server):**
   - Open Settings → Network & Internet
   - Click "Mobile hotspot"
   - Enable and select "Share over: Bluetooth or USB"
3. **On PC 2 (Client):**
   - USB network appears automatically
   - IPs will be in 192.168.137.x range

#### Option C: USB-C/A to Ethernet Adapters
1. Connect USB-to-Ethernet adapters to both PCs
2. Connect the adapters with an Ethernet cable
3. Windows detects them as Ethernet connections

#### Option D: USB Direct Link Cable
1. Purchase USB Direct Link cable (has chip inside)
2. Install driver from manufacturer
3. Connect both PCs - network created automatically

### Step 2: Configure Network (if needed)

**For direct Ethernet connection without router:**
- Computer 1: Set IP to 192.168.1.1, Subnet mask: 255.255.255.0
- Computer 2: Set IP to 192.168.1.2, Subnet mask: 255.255.255.0

**For USB-C Mobile Hotspot:**
- Automatic configuration (192.168.137.x range)
- No manual setup needed

### Step 3: Detect Connections & Find IP Addresses

**Check your connection:**
```bash
file_transfer.exe -ip
```

This will show:
- All available network connections (Ethernet, USB, WiFi)
- Connection types and status
- IP addresses for each connection

**Alternative - USB Helper Tool:**
```bash
usb_helper.exe -scan      # Scan for connections
usb_helper.exe -setup     # Show detailed USB setup guide
```

### Step 4: Transfer Files

#### On the RECEIVING computer (Server):
```bash
file_transfer.exe -r [output_filename]
```
Example:
```bash
file_transfer.exe -r received_document.pdf
```
The program will wait for an incoming connection.

#### On the SENDING computer (Client):
```bash
file_transfer.exe -s <file_to_send> <receiver_ip>
```
Example:
```bash
file_transfer.exe -s mydocument.pdf 192.168.1.1
```

## Usage Examples

### Example 1: Send a video file
**Receiver (PC1 - IP: 192.168.1.1):**
```bash
file_transfer.exe -r myvideo.mp4
```

**Sender (PC2):**
```bash
file_transfer.exe -s C:\Videos\myvideo.mp4 192.168.1.1
```

### Example 2: Send a document
**Receiver (Laptop1):**
```bash
fileMultiple connection types (Ethernet + USB)
- ✅ Fast file transfer over TCP
- ✅ Progress indicator showing transfer status
- ✅ Works with any file type and size
- ✅ Auto-detects connection types
- ✅ Displays sender's IP address on connection
- ✅ Simple command-line interface
- ✅ Built-in network scanner
- ✅ USB setup helper tool

---

## Detailed USB Setup Instructions

### Method 1: USB-C to USB-C (Windows Mobile Hotspot)

**Requirements:** Both PCs with USB-C ports, Windows 10/11

**Setup on PC 1 (Host/Server):**
1. Press `Win + I` to open Settings
2. Go to **Network & Internet** → **Mobile hotspot**
3. Toggle **Mobile hotspot** to **On**
4. Under "Share my Internet connection from," select any adapter
5. Under "Share over," select **Bluetooth** (this enables USB sharing too)
6. Connect USB-C cable between both PCs

**On PC 2 (Client):**
- Windows automatically detects USB network
- PC 2 gets IP in 192.168.137.x range
- PC 1 typically has IP 192.168.137.1

**Verify Connection:**
```bash
file_transfer.exe -ip
# Look for "USB Network" or RNDIS adapter
```

**Transfer Files:**
```bash
# PC 1 (receives):
file_transfer.exe -r

# PC 2 (sends to PC 1):
file_transfer.exe -s myfile.pdf 192.168.137.1
```

---

### Method 2: USB to Ethernet Adapters

**Requirements:** 
- 2× USB-C or USB-A to Ethernet adapters
- 1× Ethernet cable (any length)

**Setup:**
1. Connect USB-to-Ethernet adapter to each PC
2. Connect adapters with Ethernet cable
3. Windows detects as Ethernet connections
4. Set static IPs:

**On PC 1:**
- Right-click network icon → Network settings
- Click adapter → Properties → IPv4 → Properties
- Use: IP: `192.168.1.1`, Subnet: `255.255.255.0`

**On PC 2:**
- Same steps, but use IP: `192.168.1.2`

**Transfer Files:**
```bash
# PC 1:
file_transfer.exe -r

# PC 2:
file_transfer.exe -s document.pdf 192.168.1.1
```

---

### Method 3: USB Direct Link Cable

**Requirements:** USB Direct Link cable (special cable with chip - ~$15-30)

Popular brands:
- Plugable USB Direct Link Cable
- StarTech USB File Transfer Cable
- Belkin Easy Transfer Cable

**Setup:**
1. Install driver software from manufacturer
2. Connect cable between both PCs
3. Driver creates virtual network
4. Use IPs provided by driver software

**Transfer Files:**
Check driver software for assigned IPs, then:
```bash
file_transfer.exe -r              # On receiver
file_transfer.exe -s file.zip <driver_ip>  # On sender
```

---

### Method 4: Standard Ethernet Cable (No USB)

**Requirements:** Ethernet cable (Cat5e or better)

**Setup:**
1. Connect both PCs directly with Ethernet cable
2. Modern network cards auto-detect (no crossover needed)
3. Set static IPs or wait for auto-configuration (169.254.x.x)

**Manual IP Setup:**
- PC 1: `192.168.1.1`, Subnet: `255.255.255.0`
- PC 2: `192.168.1.2`, Subnet: `255.255.255.0`

---

## Connection Comparison

| Method | Speed | Setup Difficulty | Cable Cost |
|--------|-------|------------------|------------|
| Ethernet | ⭐⭐⭐⭐⭐ (1 Gbps) | Easy | $5-15 |
| USB-C Direct | ⭐⭐⭐⭐ (480 Mbps) | Medium | $5-10 |
| USB + Ethernet Adapters | ⭐⭐⭐⭐⭐ (1 Gbps) | Easy | $20-40 |
| USB Direct Link | ⭐⭐⭐ (480 Mbps) | Easy | $15-30 |

**Recommendation:** 
- **Fastest:** Ethernet cable or USB-to-Ethernet adapters
- **Most Convenient:** USB-C with Mobile Hotspot (if both PCs support it)
- **No Ethernet ports:** USB Direct Link cable
file_transfer.exe -s report.docx 192.168.1.5
```

## Features

- ✅ Fast file transfer over TCP
- ✅ Progress indicator showing transfer status
- ✅ Works with any file type and size
- ✅ Displays sender's IP address on connection
- ✅ Simple command-line interface
- ✅ Built-in IP address discovery

## Troubleshooting

### "Connection failed"
- Check that both computers are connected via cable
- Verify IP addresses are correct
- Make sure firewall allows connections on port 8888
- Ensure the receiver program is running first

### "Bind failed"
- Port 8888 may be in use
- Try closing other network applications
- Run as administrator if needed

### Can't find IP address
- Check Network Connections in Windows
- Look for the Ethernet adapter
- Use `ipconfig` command to verify IP settings

## Port Information

The program uses TCP port **8888** by default. Make sure this port is not blocked by your firewall.

## Security Note

This program is designed for direct computer-to-computer transfers on a private network. It does not include encryption. Do not use over untrusted networks without additional security measures.

## License

Free to use and modify for personal and commercial purposes.
