import re
import json

class BoardDescription:
    def __init__(self,name:str,board:dict,JSONpath:str):
        self.name = name
        self.id = board["board_id"]
        self.ip = board["board_ip"]

        # Load backend IP from general_info.json
        backend_ip = "0.0.0.0"
        try:
            with open(JSONpath + "/general_info.json") as f:
                general_info = json.load(f)
                if "addresses" in general_info and "backend" in general_info["addresses"]:
                    backend_ip = general_info["addresses"]["backend"]
        except Exception as e:
            print(f"Warning: Could not load backend IP from general_info.json: {e}")

        #Sockets:
        try:
            with open(JSONpath+"/boards/"+name+"/sockets.json") as s:
                socks = json.load(s)
                self.sockets = self.SocketsDescription(socks, self.ip, backend_ip)
        except Exception as e:
            raise Exception(f"Error in file {JSONpath}/boards/{name}/sockets.json: {e}")
        #Packets:
        self.sending_packets = []
        self.data_size = 0
        self.order_size = 0
        self.measurement_lists = []
        self.packets = {}
        for measurement in board["measurements"]:
            try:
                with open(JSONpath+"/boards/" + name + "/" + measurement) as f:
                    m = json.load(f)
                    self.measurement_lists.append(m)
            except Exception as e:
                raise Exception(f"Error in file {JSONpath}/boards/{name}/{measurement}: {e}")
        for packets in board["packets"]:
            packets_name = re.split(r'_|\.', packets)[0]
            self.packets[packets_name] = []
            try:
                with open(JSONpath+"/boards/" + name+"/" + packets) as f:
                    p= json.load(f)
            except Exception as e:
                raise Exception(f"Error in file {JSONpath}/boards/{name}/{packets}: {e}")
            i=0
            for packet in p:
                self.packets[packets_name].append(PacketDescription(packet,self.measurement_lists, packets))
                aux_sending= PacketDescription.check_for_sending(packet)
                if aux_sending is not None:
                    self.sending_packets.append(aux_sending)

                if self.packets[packets_name][i].type != "order":
                    self.data_size += 1
                else:
                    self.order_size += 1
                i += 1

        self.sending_packets = self.fix_sendind_packets(self.sending_packets)

    @staticmethod
    def fix_sendind_packets(sending_packets:list):
        fixed_packets = []
        lookup = {}
        for item in sending_packets:
            if not isinstance(item, dict):
                continue
            period = item.get("period")
            period_type = item.get("period_type")
            socket = item.get("socket")
            name = item.get("name")
            key = (period, period_type, socket)
            lookup.setdefault(key, []).append(name)

        for (period, period_type, socket), names in lookup.items():
            entry = {"period": period, "period_type": period_type, "socket": socket}
            if len(names) == 1:
                entry["name"] = names[0]
            else:
                entry["name"] = names
            fixed_packets.append(entry)

        return fixed_packets



    class SocketsDescription:
        def __init__(self, sockets: list, board_ip: str, backend_ip: str):
            self.allSockets = []
            self.ServerSockets = []
            self.Sockets = []
            self.DatagramSockets = []
            self.board_ip = board_ip
            self.backend_ip = backend_ip
            for sock in sockets:
                name = sock["name"].replace(" ", "_").replace("-", "_")
                sock_type = sock["type"]
                self.allSockets.append({"name": name, "type": sock_type})

                if sock_type == "ServerSocket":
                    self.ServerSockets.append({"name": name, "type": sock_type, "board_ip": self.board_ip, "port": sock["port"]})
                elif sock_type == "Socket":
                    remote_ip = sock["remote_ip"]
                    if remote_ip == "backend":
                        remote_ip = self.backend_ip
                    self.Sockets.append({"name": name, "type": sock_type, "board_ip": self.board_ip, "local_port": sock["local_port"], "remote_ip": remote_ip, "remote_port": sock["remote_port"]})
                elif sock_type == "DatagramSocket":
                    remote_ip = sock["remote_ip"]
                    if remote_ip == "backend":
                        remote_ip = self.backend_ip
                    self.DatagramSockets.append({"name": name, "type": sock_type, "board_ip": self.board_ip, "port": sock["port"], "remote_ip": remote_ip})



class PacketDescription:
    def __init__(self, packet:dict,measurements:list, filename:str="Unknown"):
        self.id =packet["id"]
        self.name = packet["name"].replace(" ", "_").replace("-", "_")
        self.type = packet["type"]
        self.variables = []
        self.measurements = []
        if "variables" not in packet:
            return
        for variable in packet["variables"]:
            self.variables.append(variable)
            self.measurements.append(MeasurmentsDescription(measurements,variable, filename))

    @staticmethod
    def check_for_sending(packet: dict):
        if "period" in packet and "period_type" in packet and "socket" in packet:
            name = packet["name"].replace(" ", "_").replace("-", "_")
            return {"name": name, "period": packet["period"], "period_type": packet["period_type"], "socket": packet["socket"]}

        else:
            return None
class MeasurmentsDescription:
    def __init__(self,measurements:list, variable:str, filename:str="Unknown"):
        self.id = variable
        if not hasattr(self.__class__, 'viewed_measurements'):
            self.__class__.viewed_measurements = {}
        measurement = self._MeasurementSearch(measurements,variable)

        if measurement is None:
            print(f"Measurement not found for variable: {variable} in file: {filename}\n")
            raise Exception(f"Measurement not found for variable: {variable} in file: {filename}")

        self.name = measurement["name"]
        self.type = (self._unsigned_int_correction(measurement["type"]).replace(" ", "_").replace("-", "_"))
        if self.type == "enum":
            values = []
            for value in measurement["enumValues"]:
                values.append(str(value))
            self.enum ={"name": (measurement["id"].replace(" ", "_").replace("-", "_")), "values": self._Enum_values_correction(values)}
            self.type = (measurement["id"].replace(" ", "_").replace("-", "_"))

    @staticmethod
    def _Enum_values_correction(values:list):
        for i in range(len(values)):
            values[i] = values[i].replace(" ", "_").replace("-", "_")
        return values


    @staticmethod
    def _MeasurementSearch(measurements:list, variable:str):
        if variable in MeasurmentsDescription.viewed_measurements:
            return MeasurmentsDescription.viewed_measurements[variable]
        for measurement_list in measurements:
            for measurment in measurement_list:
                if measurment["id"] == variable:
                    MeasurmentsDescription.viewed_measurements[variable] = measurment
                    return measurment
        return None


    @staticmethod
    def _unsigned_int_correction(type:str):
        aux_type = type[:4]
        if aux_type == "uint":
            type += "_t"
        elif type == "float32":
            type = "float"
        elif type == "float64":
            type = "double"
        return type
