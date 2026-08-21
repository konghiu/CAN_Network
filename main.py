import tkinter as tk
from tkinter import ttk, messagebox
import time
import threading
from scapy.all import sendp, Ether, ARP, Raw, sniff

class CANDevice:
    def __init__(self, ID, can_id, name, status="ONLINE"):
        self.ID = ID    
        self.can_id = can_id 
        self.name = name          
        self.status = status
        self.is_pinging = (status == "PING")

class NetworkSimulatorApp:
    def __init__(self, root):
        self.root = root
        self.root.title("CAN-over-Ethernet Gateway Visualizer")
        self.root.geometry("1050x780")
        self.root.configure(bg="#1e1e1e")

        # Cấu hình DUY NHẤT 1 Ethernet Node Gateway
        self.src = "01:02:03:04:05:06"
        self.dst = "21:55:44:33:22:11"
        self.INTERFACE = "Ethernet"  

        # Danh sách các thiết bị CAN trong hệ thống
        self.devices = {}

        # Dynamic Styling cho Treeview
        self.style = ttk.Style()
        self.style.theme_use("default")
        self.style.configure(
            "Treeview", 
            font=("Arial", 11), 
            rowheight=32, 
            background="#2d2d2d", 
            foreground="white", 
            fieldbackground="#2d2d2d"
        )
        self.style.configure(
            "Treeview.Heading", 
            font=("Arial", 12, "bold"), 
            background="#3e3e3e", 
            foreground="white"
        )
        self.style.map("Treeview", background=[("selected", "#007acc")])

        self.setup_ui()

        # Thêm Thread lắng nghe gói tin Ethernet ngầm
        self.running = True
        self.sniff_thread = threading.Thread(target=self.start_packet_listener, daemon=True)
        self.sniff_thread.start()

    def setup_ui(self):
        # 1. Header
        title_label = tk.Label(
            self.root, 
            text=f"GIÁM SÁT THIẾT BỊ CAN QUA ETHERNET NODE ({self.src})", 
            font=("Arial", 16, "bold"), fg="#ffffff", bg="#1e1e1e"
        )
        title_label.pack(pady=15)

        # Main Split Frame
        main_frame = tk.Frame(self.root, bg="#1e1e1e")
        main_frame.pack(fill="both", expand=True, padx=15, pady=5)

        # 2. Panel trái: Danh sách thiết bị CAN
        left_frame = tk.LabelFrame(
            main_frame, text=" Bảng Quản Lý Thiết Bị CAN ", 
            fg="#4caf50", bg="#2d2d2d", font=("Arial", 12, "bold")
        )
        left_frame.pack(side="left", fill="both", expand=True, padx=5, pady=5)

        cols = ("CAN ID", "Tên Thiết Bị", "Trạng Thái")
        self.tree = ttk.Treeview(left_frame, columns=cols, show="headings", height=8)
        
        for col in cols:
            self.tree.heading(col, text=col)
            self.tree.column(col, width=140, anchor="center")
            
        self.tree.pack(fill="both", expand=True, padx=8, pady=8)
        
        # Nút Xóa thiết bị trực tiếp trên bảng
        btn_delete_tree = tk.Button(
            left_frame, text="🗑️ Xóa Thiết Bị Đang Chọn Trên Bảng", 
            bg="#f44336", fg="white", font=("Arial", 11, "bold"),
            command=self.on_delete_selected
        )
        btn_delete_tree.pack(fill="x", padx=8, pady=(0, 8))

        self.update_device_table()

        # 3. Panel phải: Điều khiển, Sửa & Gửi dữ liệu
        right_frame = tk.LabelFrame(
            main_frame, text=" Điều Khiển & Chỉnh Sửa ", 
            fg="#00bcd4", bg="#2d2d2d", font=("Arial", 12, "bold")
        )
        right_frame.pack(side="right", fill="both", padx=5, pady=5)

        # Dropdown Chọn CAN ID
        tk.Label(right_frame, text="Chọn Thiết Bị (CAN ID):", fg="white", bg="#2d2d2d", font=("Arial", 11)).pack(anchor="w", padx=12, pady=(10, 2))
        self.dev_select = ttk.Combobox(right_frame, font=("Arial", 11), state="readonly")
        self.dev_select.pack(fill="x", padx=12, pady=3)
        self.refresh_combobox()

        # Ô Nhập Tên Thiết Bị Mới
        tk.Label(right_frame, text="Tên Thiết Bị:", fg="white", bg="#2d2d2d", font=("Arial", 11)).pack(anchor="w", padx=12, pady=(8, 2))
        self.name_entry = tk.Entry(right_frame, font=("Arial", 11), bg="#1e1e1e", fg="white", insertbackground="white")
        self.name_entry.pack(fill="x", padx=12, pady=3)

        # Nút 1: Chỉ Cập Nhật Tên
        btn_update_name = tk.Button(
            right_frame, text="✏️ Cập Nhật Tên Thiết Bị", 
            bg="#ff9800", fg="white", font=("Arial", 11, "bold"),
            command=self.on_update_name_only
        )
        btn_update_name.pack(fill="x", padx=12, pady=(10, 5))

        # Nút 2: Gửi Khung Tin ARP (Nút cũ được giữ lại)
        btn_send_arp = tk.Button(
            right_frame, text="📡 Bật / Tắt Drive", 
            bg="#2196f3", fg="white", font=("Arial", 11, "bold"),
            command=self.on_send_ethernet
        )
        btn_send_arp.pack(fill="x", padx=12, pady=5)

        # Nút 3: Bật / Tắt Ping (Nút mới)
        btn_toggle_ping = tk.Button(
            right_frame, text="⚡ Bật / Tắt Lệnh Ping", 
            bg="#9c27b0", fg="white", font=("Arial", 11, "bold"),
            command=self.on_toggle_ping
        )
        btn_toggle_ping.pack(fill="x", padx=12, pady=(5, 10))

        # 4. Bottom Frame: Bảng Log
        bottom_frame = tk.LabelFrame(
            self.root, text=f" Nhật Ký Khung Tin Ethernet Gửi/Nhận ", 
            fg="#ff9800", bg="#2d2d2d", font=("Arial", 12, "bold")
        )
        bottom_frame.pack(fill="both", expand=True, padx=15, pady=10)

        self.log_text = tk.Text(bottom_frame, bg="#121212", fg="#00ff00", font=("Consolas", 11), height=10)
        self.log_text.pack(fill="both", expand=True, padx=8, pady=8)

        # Sự kiện click hàng trên bảng
        self.tree.bind("<<TreeviewSelect>>", self.on_tree_select)
        
    # ==================== BẮT VÀ XỬ LÝ GÓI TIN ETHERNET ====================
    def start_packet_listener(self):
        try:
            sniff(iface=self.INTERFACE, filter="ether proto 0x0806", prn=self.process_received_packet, store=0)
        except Exception as e:
            print(f"Lỗi Listener: {e}")

    def process_received_packet(self, packet):
        if packet.haslayer(ARP):
            raw_bytes = bytes(packet)
            raw_id = raw_bytes[14]
            state_val = raw_bytes[15]
            analog_val = raw_bytes[16]
            if raw_id != 0:
                can_id = f"0x{raw_id:02X}"

                state_map = {0: "OFFLINE", 1: "ONLINE", 2: "PING", 3: "STOP PING"}
                status_str = state_map.get(state_val, "OFFLINE")

                self.root.after(0, self.handle_device_update, raw_id, can_id, status_str, analog_val)

    def handle_device_update(self, ID, can_id, status_str, analog_val):
        device_exist = self.devices.get(can_id)

        if device_exist is None:
            new_dev = CANDevice(ID, can_id, f"DEV-{can_id}", status_str)
            self.devices[can_id] = new_dev
            self.update_device_table()
            self.refresh_combobox()
            
            timestamp = time.strftime("%H:%M:%S")
            log_entry = (
                f"[{timestamp}] === RECEIVE ARP: NEW DEVICE ADDED ===\n"
                f" [CAN ID] {can_id} | Name: DEV-{can_id} | Status: {status_str} | Analog: {analog_val}\n"
                f"------------------------------------------------------------------------------------------\n"
            )
            self.log_text.insert("1.0", log_entry)
        elif device_exist.status != status_str:
            device_exist.status = status_str
            device_exist.is_pinging = (status_str == "PING")
            self.update_device_table()
            
            timestamp = time.strftime("%H:%M:%S")
            log_entry = (
                f"[{timestamp}] === RECEIVE ARP: STATUS UPDATED ===\n"
                f" [CAN ID] {can_id} | New Status: {status_str}\n"
                f"------------------------------------------------------------------------------------------\n"
            )
            self.log_text.insert("1.0", log_entry)

    def on_tree_select(self, event):
        """Đồng bộ form bên right_frame khi click chọn dòng trên bảng"""
        selected_item = self.tree.selection()
        if not selected_item:
            return

        row_values = self.tree.item(selected_item[0])['values']
        can_id = str(row_values[0])
        self.sync_form_to_selected_dev(can_id)

    def sync_form_to_selected_dev(self, can_id):
        if can_id in self.devices:
            dev = self.devices[can_id]
            self.dev_select.set(dev.can_id)
            
            # Cập nhật ô nhập tên
            self.name_entry.delete(0, tk.END)
            self.name_entry.insert(0, dev.name)

    def refresh_combobox(self):
        keys = list(self.devices.keys())
        if len(keys) == 0: return
        self.dev_select['values'] = keys
        if keys:
            if self.dev_select.get() not in keys:
                self.dev_select.current(0)
        else:
            self.dev_select.set("")
            self.name_entry.delete(0, tk.END)

    def update_device_table(self):
        for item in self.tree.get_children():
            self.tree.delete(item)
        for dev in self.devices.values():
            self.tree.insert("", "end", values=(dev.can_id, dev.name, dev.status))

    def on_delete_selected(self):
        selected_item = self.tree.selection()
        if not selected_item:
            messagebox.showwarning("Cảnh báo", "Vui lòng click chọn 1 hàng trong bảng để xóa!")
            return
        
        can_id = str(self.tree.item(selected_item[0])['values'][0])
        self.delete_device(can_id)

    def delete_device(self, can_id):
        if can_id in self.devices:
            dev_name = self.devices[can_id].name
            del self.devices[can_id]
            
            self.update_device_table()
            self.refresh_combobox()
            
            timestamp = time.strftime("%H:%M:%S")
            log_entry = (
                f"[{timestamp}] === REMOVE CAN DEVICE COMMAND SENT ===\n"
                f" [ETH DEST] Target MAC: {self.dst}\n"
                f" [CAN PAYLOAD] Command: REMOVE_NODE | CAN_ID: {can_id} ({dev_name})\n"
                f"------------------------------------------------------------------------------------------\n"
            )
            self.log_text.insert("1.0", log_entry)
            messagebox.showinfo("Thành công", f"Đã xóa thiết bị CAN ID: {can_id} ({dev_name})")

    def on_update_name_only(self):
        can_id = self.dev_select.get()
        if not can_id or can_id not in self.devices:
            messagebox.showwarning("Lỗi", "Vui lòng chọn một thiết bị hợp lệ!")
            return

        new_name = self.name_entry.get().strip()
        if not new_name:
            messagebox.showwarning("Lỗi", "Tên thiết bị không được để trống!")
            return

        # Cập nhật thông tin mới vào đối tượng
        device = self.devices[can_id]
        device.name = new_name
        
        # Cập nhật lại giao diện bảng
        self.update_device_table()

    def on_send_ethernet(self):
        can_id = self.dev_select.get()
        if not can_id or can_id not in self.devices:
            messagebox.showwarning("Lỗi", "Vui lòng chọn một thiết bị hợp lệ!")
            return

        device = self.devices[can_id]
        payload = bytes([device.ID, 1])
        packet = Ether(dst=self.dst, src=self.src, type=0x0806) / Raw(load=payload)
        sendp(packet, iface=self.INTERFACE, verbose=False)
        
        timestamp = time.strftime("%H:%M:%S")
        log_entry = (
            f"[{timestamp}] === TRANSMITTING CAN DEVICE OVER ETHERNET ===\n"
            f" [ETH HEADER]  Dst MAC: {self.dst} | Proto: 0x0806 (ARP/Raw)\n"
            f" [CAN PAYLOAD] Frame ID: {device.can_id} | Name: {device.name} | Status: {device.status}\n"
            f" [RAW BYTES]   {payload.hex()}\n"
            f"------------------------------------------------------------------------------------------\n"
        )

        self.log_text.insert("1.0", log_entry)

    def on_toggle_ping(self):
        can_id = self.dev_select.get()
        if not can_id or can_id not in self.devices:
            messagebox.showwarning("Lỗi", "Vui lòng chọn một thiết bị hợp lệ!")
            return

        device = self.devices[can_id]
        payload = bytes([device.ID, 2])
        packet = Ether(dst=self.dst, src=self.src, type=0x0806) / Raw(load=payload)
        sendp(packet, iface=self.INTERFACE, verbose=False)

if __name__ == "__main__":
    root = tk.Tk()
    app = NetworkSimulatorApp(root)
    root.mainloop()