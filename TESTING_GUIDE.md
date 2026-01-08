# 🧪 File Transfer Testing Guide - C-to-C Cable Connection

## Prerequisites
✅ Two PCs connected via C-to-C (USB-C to USB-C or Ethernet cable)
✅ FileTransferApp.exe built and ready
✅ Both PCs on the same network or direct connection

## Step-by-Step Testing Instructions

### 🖥️ **PC 1 (RECEIVER) - Setup First**

1. **Launch the Application**
   - Double-click `FileTransferApp.exe`
   - Main window appears

2. **Open Receiver Window**
   - Click **"📥 RECEIVE FILES"** button
   - Receiver window opens

3. **Start Listening**
   - Click **"🚀 Start Listening"** button
   - Watch the connection status change:
     - 🔴 Server Not Started → 🟡 Waiting for Connection...
   
4. **Note Your IP Address**
   - Your IP will be displayed in the large text box
   - Example: `192.168.1.100` or `169.254.x.x` (for direct cable)
   - **WRITE THIS DOWN** - you'll need it for PC 2!

5. **Status Should Show:**
   ```
   ✅ Ready. Click 'Start Listening' to wait for incoming files.
   🚀 Starting server...
   ⏳ Waiting for incoming connection...
   📡 Your IP: [YOUR IP ADDRESS]
   👂 Server listening on port 8888...
   ```

---

### 💻 **PC 2 (SENDER) - Setup Second**

1. **Launch the Application**
   - Double-click `FileTransferApp.exe` on the second PC
   - Main window appears

2. **Open Sender Window**
   - Click **"📤 SEND FILES"** button
   - Sender window opens

3. **Scan Network (Optional but Recommended)**
   - Click **"🔍 Scan Network"** button
   - You should see available network adapters
   - Look for the C-to-C cable connection (usually Ethernet or USB adapter)

4. **Enter Receiver's IP Address**
   - In the "Target PC IP Address" field
   - Enter the IP you wrote down from PC 1
   - Example: `192.168.1.100`

5. **Select a Test File**
   - Click **"📂 Browse..."** button
   - Choose a small file (e.g., a text file or image)
   - **For first test, use a small file (< 10 MB)**

6. **Start Transfer**
   - Click **"▶️ Start Transfer"** button
   - Watch the connection status change:
     - 🔴 Not Connected → 🔵 Connecting... → 🟢 Connected

---

## 📊 What You Should See

### On SENDER (PC 2):
```
🔵 Connecting...
✨ Starting file transfer...
🔄 Connecting to 192.168.1.100...
✅ Connected successfully!
🟢 Connected to 192.168.1.100
📤 Sending file: testfile.txt
🟢 Transferring Data...
[Progress bar moves from 0% to 100%]
🎉 File sent successfully!
🟢 Transfer Complete!
```

### On RECEIVER (PC 1):
```
🤝 Connection from: 192.168.1.100
🟢 Connected from 192.168.1.100
📥 Receiving file: testfile.txt
🟢 Receiving: testfile.txt
[Progress bar moves from 0% to 100%]
🎉 File received successfully!
💾 Saved to: C:\Users\[YourName]\Desktop\testfile.txt
🟢 Transfer Complete!
```

---

## 🔧 Troubleshooting

### Problem: "Connection Failed"
**Possible Causes:**
1. ❌ Receiver not started yet → Start receiver FIRST
2. ❌ Wrong IP address → Double-check the IP from receiver window
3. ❌ Firewall blocking port 8888 → Add firewall exception (see below)
4. ❌ Cable not properly connected → Check physical connection

### Problem: "Bind Failed! Port may be in use"
**Solution:**
- Another program is using port 8888
- Close any other file transfer or server applications
- Or change PORT in the code (line 23) and rebuild

### Problem: Can't see network adapter
**Solution:**
- Ensure both PCs have network sharing enabled
- Check if cable supports data transfer (some USB-C cables are charge-only)
- For direct Ethernet: You may need to set static IPs manually

### Problem: IP shows as "No Active Network Connection"
**For Direct C-to-C Cable Connection:**
1. Go to Network Settings on both PCs
2. Find the Ethernet/USB adapter
3. Manually assign IP addresses:
   - PC 1: `192.168.137.1` / Subnet: `255.255.255.0`
   - PC 2: `192.168.137.2` / Subnet: `255.255.255.0`
4. Restart the receiver application

---

## 🔥 Windows Firewall Configuration

If connection fails, add firewall exception:

1. Open **Windows Defender Firewall with Advanced Security**
2. Click **Inbound Rules** → **New Rule**
3. Select **Port** → Next
4. Choose **TCP** → Specific local ports: **8888** → Next
5. Select **Allow the connection** → Next
6. Check all profiles → Next
7. Name: "File Transfer App" → Finish

**OR use PowerShell (Run as Administrator):**
```powershell
New-NetFirewallRule -DisplayName "File Transfer App" -Direction Inbound -LocalPort 8888 -Protocol TCP -Action Allow
```

---

## ✅ Success Checklist

- [ ] Receiver shows IP address
- [ ] Receiver status: "🟡 Waiting for Connection..."
- [ ] Sender has correct IP entered
- [ ] Sender selected a file
- [ ] Connection status changes to 🟢 on both PCs
- [ ] Progress bars move
- [ ] File appears on Desktop
- [ ] Both show "Transfer Complete!"

---

## 🎯 Quick Test Sequence

1. **PC 1**: Run app → Receive Files → Start Listening → Note IP
2. **PC 2**: Run app → Send Files → Enter IP from PC 1 → Browse file → Start Transfer
3. **Watch**: Both PCs should show green connection status
4. **Verify**: Check Desktop on PC 1 for received file

---

## 📝 Connection Types Expected

### For USB-C to USB-C Cable (Direct):
- IP Range: `169.254.x.x` (APIPA - Automatic Private IP Addressing)
- OR manually set: `192.168.x.x`

### For Ethernet Cable (Direct):
- Crossover cable or auto-MDIX capable NICs
- IP Range: `192.168.x.x` or `169.254.x.x`

### For Network Switch/Router:
- Both get IPs from DHCP
- IP Range: Depends on router (e.g., `192.168.1.x`)

---

## 🚨 Important Notes

1. **START RECEIVER FIRST** - Always start the receiving PC before trying to send
2. **Use Small Files First** - Test with small files before large transfers
3. **Port 8888** - This application uses TCP port 8888
4. **Desktop Saves** - Received files automatically go to Desktop
5. **Same Filename** - If file exists, it will be overwritten

---

## 📞 Still Having Issues?

Check these:
1. Both PCs running the correct version of the app
2. Windows Firewall temporarily disabled for testing
3. Network adapter enabled and working
4. Cable is data-capable (not charge-only)
5. Try pinging between PCs: `ping [OTHER_PC_IP]`

---

**Happy Testing! 🚀**

If everything works, you should see green connection indicators and successful file transfers!
