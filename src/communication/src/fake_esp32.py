
import socket
import time

HOST = "127.0.0.1"
PORT = 5005

TIMEOUT = 0.5

esp = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
esp.bind((HOST, PORT))

# Agar recvfrom tidak menunggu selamanya
esp.settimeout(0.1)

last_sequence = None
last_command_time = None

rpm = 0
connected = False

print("================================")
print("       FAKE ESP32 AKTIF")
print("================================")

while True:

    try:
        data, client_address = esp.recvfrom(1024)

        message = data.decode("utf-8").strip()
        parts = message.split(",")

        if len(parts) != 5 or parts[0] != "CMD":
            print("Paket tidak valid")
            continue

        sequence = int(parts[1])
        steering = float(parts[2])
        speed = float(parts[3])
        mode = parts[4]

        # Validasi sequence
        if last_sequence is not None and sequence <= last_sequence:
            print("Paket lama diabaikan")
            continue

        # Memperbarui waktu command terakhir
        last_sequence = sequence
        last_command_time = time.monotonic()

        if not connected:
            print("\nSTATUS: CONNECTED")

        connected = True

        print(f"CMD diterima: {sequence}")
        print(f"Steering: {steering}")
        print(f"Speed: {speed}")
        print(f"Mode: {mode}")

        # Simulasi telemetry
        rpm = min(rpm + 5, 120)

        response = (
            f"TEL,{sequence},{rpm},11.8,RUNNING,0"
        )

        esp.sendto(
            response.encode("utf-8"),
            client_address
        )

    except socket.timeout:

        # Memeriksa waktu sejak command terakhir
        if last_command_time is not None:

            elapsed = time.monotonic() - last_command_time

            if elapsed > TIMEOUT and connected:

                print("\nSTATUS: TIMEOUT")
                print("Komunikasi laptop terputus!")

                connected = False

                # Simulasi status aman
                rpm = 0
                print("Simulasi: motor masuk kondisi berhenti")
