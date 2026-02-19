from Packet_generation.Packet_descriptions import *
import json
import os
import jinja2
import sys

templates_path = "Core/Inc/Code_generation/Packet_generation"

def Generate_PacketDescription(JSONpath:str,board:str):    
    with open(JSONpath+"/boards.json") as f:
        boards = json.load(f)
    boards_name = []
    for b in boards:
        boards_name.append(b)
    if board in boards_name:
        with open(JSONpath+"/" + (boards[board])) as f:
            b = json.load(f)
            board_instance = BoardDescription(board, b,JSONpath)
            globals()[board] = board_instance
    else:
        print(f"Board {board} not found, exiting...")
        sys.exit()
    
    return boards_name
        

#--------------DataPackets.hpp generation---------------#

def Get_data_context(board:BoardDescription):
    def GenerateDataEnum(board:BoardDescription):
        Enums = []
        for packet in board.packets:
            for packet_instance in board.packets[packet]:
                if packet_instance.type != "order":
                    for measurement in packet_instance.measurements:
                        if hasattr(measurement, "enum")and measurement.enum not in Enums:
                            Enums.append(measurement.enum)
        return Enums
    
    
    def GenerateDataPackets(board:BoardDescription):
        Packets =[]
        totaldata = []
        for packet in board.packets:
            for packet_instance in board.packets[packet]:
                if packet_instance.type != "order":
                    tempdata = ""
                    tempdata_but_pointer = ""
                    for variable in packet_instance.variables:
                        tempdata +=(str(variable) +",")
                        tempdata_but_pointer +=("&"+str(variable) +",")
                    if tempdata.endswith(","):
                        tempdata = tempdata[:-1]  
                    if tempdata_but_pointer.endswith(","):
                        tempdata_but_pointer = tempdata_but_pointer[:-1]
                    
                    packet_variables = []
                    for measurement in packet_instance.measurements:
                        packet_variables.append({
                            "name": measurement.id.replace(" ", "_").replace("-", "_"),
                            "type": measurement.type
                        })

                    aux_packet = {"name": packet_instance.name, "data":tempdata_but_pointer.replace(" ", "_").replace("-", "_") , "id": packet_instance.id, "variables": packet_variables}
                    Packets.append(aux_packet)
                    for measurement in packet_instance.measurements:
                        aux_data = {"type": measurement.type, "name": measurement.id.replace(" ", "_").replace("-", "_")}
                        if not any(x["name"] == aux_data["name"] for x in totaldata):
                            totaldata.append(aux_data)
        
        return Packets,totaldata
    
    packets,data = GenerateDataPackets(board)
    
    def GenerateGroupedSendingPackets(board: BoardDescription):
        datagram_sockets = [s["name"] for s in board.sockets.DatagramSockets]
        grouped_lookup = {}

        for packet in board.sending_packets:
            socket_name = packet["socket"]
            if socket_name not in datagram_sockets:
                continue

            period = packet["period"]
            period_type = packet["period_type"]
            names = packet["name"]
            
            key = (period, period_type)
            if key not in grouped_lookup:
                grouped_lookup[key] = []
                
            if isinstance(names, list):
                for name in names:
                    grouped_lookup[key].append({"socket": socket_name, "name": name})
            else:
                grouped_lookup[key].append({"socket": socket_name, "name": names})
        
        grouped_list = []
        for (period, period_type), items in grouped_lookup.items():
            grouped_list.append({
                "period": period,
                "period_type": period_type,
                "packets": items
            })
        return grouped_list

    context = {
        "board": board.name,
        "enums": GenerateDataEnum(board),
        "packets" : packets,
        "data": data,
        "size": board.order_size,
        "Sockets":board.sockets.Sockets,
        "DatagramSockets":board.sockets.DatagramSockets,
        "DatagramSocketNames": [s["name"] for s in board.sockets.DatagramSockets],
        "sending_packets": GenerateGroupedSendingPackets(board),
    }
    return context

def Generate_DataPackets_hpp(board_input:str):
    data_packets_path = "Core/Inc/Communications/Packets/DataPackets.hpp"
    board_instance = globals()[board_input]
    if board_instance.data_size == 0:
        if os.path.exists(data_packets_path):
            os.remove(data_packets_path)
        return    
  
    env= jinja2.Environment(loader=jinja2.FileSystemLoader(templates_path))
    template = env.get_template("DataTemplate.hpp")
    context = Get_data_context(board_instance)

    
    with open(data_packets_path,"w") as Output:
        Output.write(template.render(context))
            
#--------------OrderPackets.hpp generation---------------#

def Get_order_context(board:BoardDescription):
    def GenerateOrderEnum(board:BoardDescription):
        Enums = []
        for packet in board.packets:
            for packet_instance in board.packets[packet]:
                if packet_instance.type == "order":
                    for measurement in packet_instance.measurements:
                        if hasattr(measurement, "enum") and measurement.enum not in Enums:
                            Enums.append(measurement.enum)
        return Enums
    
    
    def GenerateOrderPackets(board:BoardDescription):
        Packets =[]
        totaldata = []
        for packet in board.packets:
            for packet_instance in board.packets[packet]:
                if packet_instance.type == "order":
                    tempdata = ""
                    tempdata_but_pointer = ""
                    for variable in packet_instance.variables:
                        tempdata +=(str(variable) +",")
                        tempdata_but_pointer +=("&"+str(variable) +",")
                    if tempdata.endswith(","):
                        tempdata = tempdata[:-1] 
                        tempdata_but_pointer = tempdata_but_pointer[:-1] 

                    packet_variables = []
                    for measurement in packet_instance.measurements:
                        packet_variables.append({
                            "name": measurement.id.replace(" ", "_").replace("-", "_"),
                            "type": measurement.type
                        })

                    aux_packet = {"name": packet_instance.name, "data":tempdata_but_pointer , "id": packet_instance.id, "variables": packet_variables}
                    Packets.append(aux_packet)
                    for measurement in packet_instance.measurements:
                        aux_data = {"type": measurement.type, "name": measurement.id}
                        if not any(x["name"] == aux_data["name"] for x in totaldata):
                            totaldata.append(aux_data)
        
        return Packets,totaldata
    
    
    packets,data = GenerateOrderPackets(board)
    context = {
        "board": board.name,
        "enums": GenerateOrderEnum(board),
        "packets" : packets,
        "data": data,
        "size": board.order_size,
        "ServerSockets":board.sockets.ServerSockets,
        "Sockets":board.sockets.Sockets,
    }
    return context

def Generate_OrderPackets_hpp(board_input:str):
    order_packets_path = "Core/Inc/Communications/Packets/OrderPackets.hpp"
    board_instance = globals()[board_input]
    if board_instance.order_size == 0:
        if os.path.exists(order_packets_path):
            os.remove(order_packets_path)
        return    
  
    env= jinja2.Environment(loader=jinja2.FileSystemLoader(templates_path))
    template = env.get_template("OrderTemplate.hpp")
    context = Get_order_context(board_instance)

    
    with open(order_packets_path,"w") as Output:
        Output.write(template.render(context))


