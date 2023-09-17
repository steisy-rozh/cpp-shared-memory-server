import ctypes
import time


def start_chat():
    ichat = ctypes.WinDLL("./IpcChat.dll")
    ichat.read_message.argtypes = [ctypes.POINTER(ctypes.c_char_p)]
    ichat.read_message.restype = ctypes.c_int
    return ichat


def read_message(ch):
    message_ptr = ctypes.c_char_p(ctypes.addressof(ctypes.create_string_buffer(1000)))
    try:
        index = ch.read_message(ctypes.byref(message_ptr))
        if index > 0 and message_ptr.value is not None:
            print(f"[{index}]: {message_ptr.value.decode('utf-8')}")
    except Exception as e:
        print(f"could not read message due to error {e}")


if __name__ == '__main__':
    chat = start_chat()
    while True:
        read_message(chat)
        time.sleep(5)
