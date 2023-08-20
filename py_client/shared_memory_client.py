import ctypes
import mmap

memory_name = 'Local\\CppToPythonChat'
max_entities = 10
max_length = 1000
index = 0


class Message(ctypes.Structure):
    _fields_ = [
        ('index', ctypes.c_int32),
        ('text', ctypes.c_char * max_length),
    ]


class TransferData(ctypes.Structure):
    _fields_ = [
        ('Messages', Message * max_entities),
    ]


def read_from_memory():
    d_shm = mmap.mmap(-1, ctypes.sizeof(TransferData), memory_name, access=mmap.ACCESS_READ)
    if d_shm:
        try:
            print(d_shm.readline())
            read = TransferData.from_buffer_copy(d_shm)
            print_data(read.Messages)
        finally:
            d_shm.close()
    else:
        raise NotImplementedError("could not find a messages due to invalid data structure.")


def print_data(messages):
    for message in messages:
        print(f"got message with index {message.index}")
        print(f"the message is {message.text.decode('utf-8', 'ignore')}")


if __name__ == '__main__':
    read_from_memory()
