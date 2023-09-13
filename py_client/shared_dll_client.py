import ctypes
import time


def start_chat():
    ichat = ctypes.WinDLL("./IpcChat.dll")
    ichat.read_message.argtypes = (ctypes.c_char_p, )
    ichat.read_message.restype = ctypes.c_int32
    return ichat


def read_message(chat):
    message = ctypes.c_char_p()
    try:
        index = chat.read_message(message)
        if index > 0:
            print(f"[{index}]: {message}")
    except Exception as e:
        print(f"could not read message due to error {e}")


if __name__ == '__main__':
    chat = start_chat()
    while True:
        read_message(chat)
        time.sleep(5)
