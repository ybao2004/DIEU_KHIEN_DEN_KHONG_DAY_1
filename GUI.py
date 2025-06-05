import tkinter as tk
from tkinter import ttk, scrolledtext, messagebox
import serial
import serial.tools.list_ports
import threading
import time

class ESP8266Controller:
    def __init__(self, root):
        self.root = root
        self.root.title("Controller_ver-1")
        self.root.geometry("{}x{}+{}+{}".format(800,600,50,50))
        self.root.iconbitmap('icon.ico')
        self.root.configure(bg="#877777")

        self.serial_conn = None
        self.wifi_list = []
        self.connected = False
        self.listening = False

        self.create_gui()
        self.start_serial_listener()

    def create_gui(self):
        main_frame = ttk.Frame(self.root, padding="10")
        main_frame.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))

        left_frame = ttk.LabelFrame(main_frame, text="Cài đặt kết nối", padding="10")
        left_frame.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S), padx=(0, 10))

        ttk.Label(left_frame, text="Cổng COM:").grid(row=0, column=0, sticky=tk.W, pady=2)
        self.com_var = tk.StringVar()
        self.com_combo = ttk.Combobox(left_frame, textvariable=self.com_var, width=15)
        self.com_combo.grid(row=0, column=1, sticky=(tk.W, tk.E), pady=2, padx=(5, 0))

        ttk.Label(left_frame, text="Baudrate:").grid(row=1, column=0, sticky=tk.W, pady=2)
        self.baud_var = tk.StringVar(value="115200")
        self.baud_combo = ttk.Combobox(left_frame, textvariable=self.baud_var, 
                                      values=["9600", "19200", "38400", "57600", "115200"], width=15)
        self.baud_combo.grid(row=1, column=1, sticky=(tk.W, tk.E), pady=2, padx=(5, 0))

        self.connect_btn = ttk.Button(left_frame, text="Kết nối ESP8266", command=self.connect_serial)
        self.connect_btn.grid(row=2, column=0, columnspan=2, pady=10, sticky=(tk.W, tk.E))

        self.scan_btn = ttk.Button(left_frame, text="Quét WiFi", command=self.scan_wifi, state=tk.DISABLED)
        self.scan_btn.grid(row=3, column=0, columnspan=2, pady=5, sticky=(tk.W, tk.E))

        ttk.Label(left_frame, text="WiFi:").grid(row=4, column=0, sticky=tk.W, pady=2)
        self.wifi_var = tk.StringVar()
        self.wifi_combo = ttk.Combobox(left_frame, textvariable=self.wifi_var, width=15, state="readonly")
        self.wifi_combo.grid(row=4, column=1, sticky=(tk.W, tk.E), pady=2, padx=(5, 0))

        ttk.Label(left_frame, text="Mật khẩu:").grid(row=5, column=0, sticky=tk.W, pady=2)
        self.password_var = tk.StringVar()
        self.password_entry = ttk.Entry(left_frame, textvariable=self.password_var, show="*", width=15, state=tk.DISABLED)
        self.password_entry.grid(row=5, column=1, sticky=(tk.W, tk.E), pady=2, padx=(5, 0))

        ttk.Label(left_frame, text="IP Server:").grid(row=6, column=0, sticky=tk.W, pady=2)
        self.ip_var = tk.StringVar(value="192.168.4.1")
        self.ip_entry = ttk.Entry(left_frame, textvariable=self.ip_var, width=15, state=tk.DISABLED)
        self.ip_entry.grid(row=6, column=1, sticky=(tk.W, tk.E), pady=2, padx=(5, 0))

        ttk.Label(left_frame, text="Cổng:").grid(row=7, column=0, sticky=tk.W, pady=2)
        self.port_var = tk.StringVar(value="100")
        self.port_entry = ttk.Entry(left_frame, textvariable=self.port_var, width=15, state=tk.DISABLED)
        self.port_entry.grid(row=7, column=1, sticky=(tk.W, tk.E), pady=2, padx=(5, 0))

        self.connect_wifi_btn = ttk.Button(left_frame, text="Kết nối WiFi & Server", 
                                          command=self.connect_wifi, state=tk.DISABLED)
        self.connect_wifi_btn.grid(row=8, column=0, columnspan=2, pady=10, sticky=(tk.W, tk.E))

        self.status_var = tk.StringVar(value="Chưa kết nối")
        ttk.Label(left_frame, textvariable=self.status_var, foreground="red").grid(row=9, column=0, columnspan=2, pady=5)

        center_frame = ttk.Frame(main_frame)
        center_frame.grid(row=0, column=1, sticky=(tk.W, tk.E, tk.N, tk.S), padx=(0, 10))

        terminal_frame = ttk.LabelFrame(center_frame, text="Terminal ESP8266", padding="5")
        terminal_frame.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S), pady=(0, 10))

        self.terminal = scrolledtext.ScrolledText(terminal_frame, height=20, width=50)
        self.terminal.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))

        send_input_frame = ttk.LabelFrame(center_frame, text="Lệnh gửi & Nhập lệnh", padding="5")
        send_input_frame.grid(row=1, column=0, sticky="ew")

        self.send_display = scrolledtext.ScrolledText(send_input_frame, height=6, width=50, state=tk.DISABLED)
        self.send_display.grid(row=0, column=0, columnspan=2, sticky=(tk.W, tk.E, tk.N))

        self.custom_var = tk.StringVar()
        self.custom_entry = ttk.Entry(send_input_frame, textvariable=self.custom_var, width=50, state=tk.DISABLED)
        self.custom_entry.grid(row=1, column=0, sticky=(tk.W, tk.E), pady=(5, 0))
        self.custom_entry.bind('<Return>', lambda e: self.send_command(self.custom_var.get()))

        self.custom_btn = ttk.Button(send_input_frame, text="Gửi", 
                                     command=lambda: self.send_command(self.custom_var.get()), state=tk.DISABLED)
        self.custom_btn.grid(row=1, column=1, padx=(5, 0), pady=(5, 0))

        send_input_frame.columnconfigure(0, weight=1)

        right_frame = ttk.LabelFrame(main_frame, text="Điều khiển", padding="10")
        right_frame.grid(row=0, column=2, sticky=(tk.W, tk.E, tk.N, tk.S))

        buttons = [
            ("ON", "on"), ("OFF", "off"), ("RESET", "reset"), ("INFOR", "infor"), ("RANDOM", "random"),
            ("F0", "f0"), ("F1", "f1"), ("F2", "f2")
        ]

        row, col = 0, 0
        for text, cmd in buttons:
            btn = ttk.Button(right_frame, text=text, width=8,
                             command=lambda c=cmd: self.send_command(c), state=tk.DISABLED)
            btn.grid(row=row, column=col, padx=2, pady=2)
            setattr(self, f"{cmd}_btn", btn)
            col += 1
            if col > 1:
                col = 0
                row += 1

        ttk.Label(right_frame, text="Ghi chú:").grid(row=row+1, column=0, columnspan=2, sticky=tk.W, pady=(10, 2))
        note_text = "Sử dụng lệnh tùy chỉnh để:\n- Nhập lệnh đơn\n- Nhập nhiều lệnh cùng lúc\n- Gửi lệnh phức tạp"
        ttk.Label(right_frame, text=note_text, font=('TkDefaultFont', 8), justify=tk.LEFT).grid(row=row+2, column=0, columnspan=2, sticky=tk.W, pady=2)

        self.root.columnconfigure(0, weight=1)
        self.root.rowconfigure(0, weight=1)
        main_frame.columnconfigure(1, weight=1)
        main_frame.rowconfigure(0, weight=1)
        center_frame.columnconfigure(0, weight=1)
        center_frame.rowconfigure(0, weight=1)
        center_frame.rowconfigure(1, weight=0)
        terminal_frame.columnconfigure(0, weight=1)
        terminal_frame.rowconfigure(0, weight=1)

        self.refresh_com_ports()
        
    def refresh_com_ports(self):
        ports = [port.device for port in serial.tools.list_ports.comports()]
        self.com_combo['values'] = ports
        if ports:
            self.com_var.set(ports[0])
    
    def connect_serial(self):
        if not self.connected:
            try:
                port = self.com_var.get()
                baud = int(self.baud_var.get())
                self.serial_conn = serial.Serial(port, baud, timeout=1)
                time.sleep(2)
                
                self.connected = True
                self.status_var.set("Đã kết nối Serial")
                self.connect_btn.config(text="Ngắt kết nối")
                self.enable_controls(True)
                self.log_terminal(f"Kết nối thành công {port} @ {baud}")
                
            except Exception as e:
                messagebox.showerror("Lỗi", f"Không thể kết nối: {str(e)}")
        else:
            self.disconnect_serial()
    
    def disconnect_serial(self):
        if self.serial_conn:
            self.serial_conn.close()
            self.serial_conn = None
        self.connected = False
        self.status_var.set("Chưa kết nối")
        self.connect_btn.config(text="Kết nối ESP8266")
        self.enable_controls(False)
        self.log_terminal("Ngắt kết nối Serial")
        self.wifi_list.clear()
        self.wifi_combo['values'] = []
    
    def enable_controls(self, enabled):
        state = tk.NORMAL if enabled else tk.DISABLED
        controls = [
            self.scan_btn, self.password_entry, 
            self.ip_entry, self.port_entry, self.connect_wifi_btn,
            self.custom_entry, self.custom_btn
        ]
        
        for control in controls:
            control.config(state=state)
            
        self.wifi_combo.config(state="readonly" if enabled else tk.DISABLED)
        
        buttons = ["on", "off", "reset", "infor", "random", "f0", "f1", "f2"]
        for btn_name in buttons:
            if hasattr(self, f"{btn_name}_btn"):
                getattr(self, f"{btn_name}_btn").config(state=state)
    
    def scan_wifi(self):
        if self.serial_conn:
            self.wifi_list.clear()
            self.wifi_combo['values'] = []
            self.wifi_var.set("")
            self.log_send_command("SCAN_WIFI")
            self.serial_conn.write(b"SCAN_WIFI\n")
    
    def connect_wifi(self):
        if not self.serial_conn:
            return
            
        wifi = self.wifi_var.get()
        password = self.password_var.get()
        ip = self.ip_var.get()
        port = self.port_var.get()
        
        if not wifi:
            messagebox.showwarning("Cảnh báo", "Vui lòng chọn tên WiFi")
            return
        
        cmd = f"SET_WIFI:{wifi}:{password}:"
        self.serial_conn.write(cmd.encode() + b"\n")
        self.log_send_command(cmd)
        time.sleep(0.1)
        
        cmd = f"SET_SERVER:{ip}:{port}:"
        self.serial_conn.write(cmd.encode() + b"\n")
        self.log_send_command(cmd)
        time.sleep(0.1)
        
        self.serial_conn.write(b"CONNECT\n")
        self.log_send_command("CONNECT")
    
    def send_command(self, cmd):
        if self.serial_conn and cmd:
            self.serial_conn.write(cmd.encode() + b"\n")
            self.log_send_command(f"{cmd}")
            self.custom_var.set("")
    
    def log_send_command(self, message):
        self.send_display.config(state=tk.NORMAL)
        self.send_display.insert(tk.END, f"{time.strftime('%H:%M:%S')} -> {message}\n")
        self.send_display.see(tk.END)
        self.send_display.config(state=tk.DISABLED)
    
    def log_terminal(self, message):
        self.terminal.insert(tk.END, f"{time.strftime('%H:%M:%S')} {message}\n")
        self.terminal.see(tk.END)
    
    def start_serial_listener(self):
        self.listening = True
        self.listener_thread = threading.Thread(target=self.serial_listener, daemon=True)
        self.listener_thread.start()
    
    def serial_listener(self):
        while self.listening:
            if self.serial_conn and self.serial_conn.is_open:
                try:
                    if self.serial_conn.in_waiting:
                        line = self.serial_conn.readline().decode('utf-8', errors='ignore').strip()
                        if line:
                            self.process_serial_data(line)
                except Exception as e:
                    self.root.after(0, lambda: self.log_terminal(f"Lỗi đọc Serial: {str(e)}"))
            time.sleep(0.01)
    
    def process_serial_data(self, data):
        self.log_terminal(data)
        
        if data.startswith("GUI_WIFI:"):
            parts = data.split(":")
            if len(parts) >= 4:
                wifi_name = parts[1]
                signal = parts[2]
                security = parts[3]
                wifi_info = f"{wifi_name} ({signal}dBm, {security})"
                if wifi_name and wifi_name not in self.wifi_list:
                    self.wifi_list.append(wifi_name)
                    self.root.after(0, lambda: self.wifi_combo.configure(values=self.wifi_list))
        
        elif data == "GUI_SCAN_START":
            self.wifi_list.clear()
            self.root.after(0, lambda: self.wifi_combo.configure(values=[]))
            self.log_terminal("Đang quét mạng wifi")
        
        elif data == "GUI_SCAN_END":
            self.log_terminal("Hoành thành quét!")
            if not self.wifi_list:
                self.root.after(0, lambda: messagebox.showinfo("Thông báo", "Không tìm thấy mạng WiFi"))
        
        elif data == "GUI_SCAN_NONE":
            self.log_terminal("Không tìm thấy mạng WiFi")
            self.root.after(0, lambda: messagebox.showinfo("Thông báo", "Không tìm thấy mạng WiFi"))
        
        elif data.startswith("GUI_WIFI_CONNECTED"):
            self.status_var.set("WiFi đã kết nối")
        
        elif data.startswith("GUI_SERVER_CONNECTED"):
            self.status_var.set("Đã kết nối đầy đủ")

def main():
    root = tk.Tk()
    app = ESP8266Controller(root)
    root.mainloop()

if __name__ == "__main__":
    main()