import bzapi

def slash_command_handler(from_id: int, to_id: int, text: str) -> None:
    tokens = text.split()

    if tokens[0] == '/setPlayerParameter':
        if len(tokens) != 4:
            bzapi.send_chat_message(0, from_id, "Usage: /setPlayerParameter <client_id> <parameter> <value>")
            return True

        target_id = int(tokens[1])
        parameter = tokens[2]
        try:
            value = float(tokens[3])
        except ValueError:
            bzapi.send_chat_message(0, from_id, "Value must be a number.")
            return True

        if not bzapi.set_player_parameter(target_id, parameter, value):
            bzapi.send_chat_message(0, from_id, f"Failed to set parameter '{parameter}' for player ID {target_id}.")
        else:
            bzapi.send_chat_message(0, from_id, f"Player parameter '{parameter}' set to {value}.")
        return True
    
    elif tokens[0] == '/kill':
        if len(tokens) != 2:
            bzapi.send_chat_message(0, from_id, "Usage: /kill <player_name>")
            return True

        target_name = tokens[1]
        target_id = bzapi.get_player_by_name(target_name)


        if target_id is None:
            bzapi.send_chat_message(0, from_id, f"Player '{target_name}' not found.")
            return True

        bzapi.kill_player(target_id)
        bzapi.send_chat_message(0, from_id, f"Player '{target_name}' has been killed.")
        return True

    elif tokens[0] == '/listPlayers':
        player_list = bzapi.get_all_player_ids()
        
        for pid in player_list:
            pname = bzapi.get_player_name(pid)
            pip = bzapi.get_player_ip(pid)
            bzapi.send_chat_message(0, from_id, f"Player ID: {pid}, Name: {pname}, IP: {pip}")

        return True
    
    elif tokens[0][0] == '/':
        bzapi.send_chat_message(0, from_id, f"Unknown command: {tokens[0]}")
        return True
    
    return False

bzapi.register_callback(bzapi.event_type.CHAT, slash_command_handler)
