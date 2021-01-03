# bluetooth low energy scan
# from bluetooth.ble import DiscoveryService
# 
# service = DiscoveryService()
# devices = service.discover(2)

# for address, name in devices.items():
#     print("name: {}, address: {}".format(name, address))


import sys
# from bluetooth.ble import GATTRequester
from gattlib import GATTRequester


class Reader:

    def __init__(self, address):
        self.requester = GATTRequester(address, False)
        self.connect()
        self.request_name()

        characteristics = self.requester.discover_characteristics()
        for ch in characteristics:
            print(ch)
        
        data = self.requester.read_by_handle(0x000e)
        print(data)


    def connect(self):
        print("Connecting...", end=" ")
        sys.stdout.flush()

        self.requester.connect(True, channel_type="random")
        print("OK.")

    def request_name(self):
        data = self.requester.read_by_uuid(
            "00002a00-0000-1000-8000-00805f9b34fb")[0]
        try:
            print("Device name:", data.decode("utf-8"))
        except AttributeError:
            print("Device name:", data)
    
    def onNotification(self, args):
        pass
    



if __name__ == "__main__":
    reader = Reader("D5:C9:57:C2:25:7B")
    print("Done.")