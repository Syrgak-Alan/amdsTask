import serial
import time
import csv
import threading
import sys

PORT = 'COM9' 
BAUD = 115200

def serial_listener(ser, stop_event):
    inverse_data = []
    is_receiving_results = False

    while not stop_event.is_set():
        if ser.in_waiting:
            try:
                line = ser.readline().decode(errors='ignore').strip()
                if not line or line == ">": continue
                
                if "RESULT_START" in line:
                    is_receiving_results = True
                    inverse_data = []
                    print("\n[Python] Receiving Inverse Matrix...")
                    continue
                
                if "RESULT_END" in line:
                    is_receiving_results = False
                    save_to_csv(inverse_data)
                    print(f"[Python] Success: {len(inverse_data)} rows saved to 'inverse_result.csv'")
                    continue

                if is_receiving_results:
                    # Filter out anything that isn't a number or a minus/decimal/space
                    # This prevents "> cmd" from being parsed
                    parts = [x for x in line.split() if x.replace('.','',1).replace('-','',1).isdigit()]
                    
                    if not parts: continue 
                    
                    row = [float(x) for x in parts]
                    
                    if len(row) == 100:
                        inverse_data.append(row)
                    else:
                        print(f"[Warning] Fragmented row ignored (Length: {len(row)})")
                else:
                    print(f"Teensy: {line}")
            
            except Exception as e:
                if is_receiving_results:
                    print(f"[Data Error] {e}")

def save_to_csv(data):
    with open('inverse_result.csv', 'w', newline='') as f:
        writer = csv.writer(f)
        writer.writerows(data)

def upload_matrix(ser):
    print("[Python] Streaming CSV...")
    with open('matrix.csv', 'r') as f:
        reader = csv.reader(f)
        for row in reader:
            line_data = " ".join(row) + " "
            ser.write(line_data.encode())
            time.sleep(0.005)

def run_manager():
    try:
        ser = serial.Serial(PORT, BAUD, timeout=0.1)
        time.sleep(2)
        ser.reset_input_buffer()

        stop_event = threading.Event()
        listener_thread = threading.Thread(target=serial_listener, args=(ser, stop_event), daemon=True)
        listener_thread.start()

        print("--- Automated Matrix Task Started ---")
        
        print("[Python] Commanding Load...")
        ser.write(b"cmd load\n")
        time.sleep(0.5) 
        
        upload_matrix(ser)
        
        print("[Python] Commanding Start...")
        ser.write(b"cmd start\n")

        print("\n--- Manual Terminal Mode Active ---")
        print("Type 'cmd frequency=X' to change LED or 'exit' to quit.")
        while True:
            user_input = input("> ")
            if user_input.lower() == 'exit':
                break

            if user_input.lower() == 'upload':
                upload_matrix(ser)

            ser.write((user_input + "\n").encode())

        stop_event.set()
        ser.close()

    except Exception as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    run_manager()