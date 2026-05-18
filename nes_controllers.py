import pygame
import socket
import time

PICO_IP = "192.168.4.1"
PORT = 8888

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

pygame.init()
pygame.joystick.init()

print("[BOOT] pygame initialized")
print("[BOOT] joystick subsystem initialized")

if pygame.joystick.get_count() < 1:
    print("[ERROR] No USB gamepad found")
    raise SystemExit

js = pygame.joystick.Joystick(0)
js.init()

print("[BOOT] Controller connected:", js.get_name())
print("[BOOT] Axes:", js.get_numaxes(), "Buttons:", js.get_numbuttons())

dead = 0.35
last_move = None
SERVO_BUTTON = 0
servo_button_held = False

print("[CONFIG] deadzone =", dead)
print("[CONFIG] servo button index =", SERVO_BUTTON)
print("[CONFIG] Pico target =", PICO_IP, "port", PORT)

def send_raw(cmd):
    print("[UDP] RAW SEND ->", cmd)
    sock.sendto(cmd.encode(), (PICO_IP, PORT))

def send_move(cmd):
    global last_move
    if cmd != last_move:
        print("[UDP] MOVE SEND ->", cmd, "(previous:", last_move, ")")
        sock.sendto(cmd.encode(), (PICO_IP, PORT))
        last_move = cmd

while True:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            print("[EVENT] pygame quit")
            raise SystemExit

        if event.type == pygame.JOYBUTTONDOWN:
            print("[EVENT] BUTTON DOWN ->", event.button)
            if event.button == SERVO_BUTTON and not servo_button_held:
                print("[SERVO] trigger button pressed, sending S once")
                send_raw("S")
                servo_button_held = True

        if event.type == pygame.JOYBUTTONUP:
            print("[EVENT] BUTTON UP ->", event.button)
            if event.button == SERVO_BUTTON:
                servo_button_held = False
                print("[SERVO] trigger button released, ready for next press")

    x = js.get_axis(0) if js.get_numaxes() > 0 else 0.0
    y = js.get_axis(1) if js.get_numaxes() > 1 else 0.0

    if abs(y) > abs(x) and abs(y) > dead:
        cmd = "B" if y > 0 else "F"
    elif abs(x) > dead:
        cmd = "R" if x > 0 else "L"
    else:
        cmd = "X"

    send_move(cmd)
    time.sleep(0.05)