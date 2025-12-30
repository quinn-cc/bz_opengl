import bz_plugins


def slash_command_handler(from_id: int, to_id: int, text: str) -> None:
    tokens = text.split()

    if tokens[0] == '/set':
        if len(tokens) != 3:
            bz_plugins.send_chat_message(0, from_id, "Usage: /set <setting> <value>")
            return True

        setting = tokens[1]
        try:
            value = float(tokens[2])
        except ValueError:
            bz_plugins.send_chat_message(0, from_id, "Value must be a number.")
            return True

        bz_plugins.set_world_setting(setting, value)
        bz_plugins.send_chat_message(0, from_id, f"World setting '{setting}' set to {value}.")
        return True
    
    elif tokens[0] == '/kill':
        if len(tokens) != 2:
            bz_plugins.send_chat_message(0, from_id, "Usage: /kill <player_name>")
            return True

        target_name = tokens[1]
        target_id = bz_plugins.get_player_by_name(target_name)


        if target_id == 0:
            bz_plugins.send_chat_message(0, from_id, f"Player '{target_name}' not found.")
            return True

        bz_plugins.kill_player(target_id)
        bz_plugins.send_chat_message(0, from_id, f"Player '{target_name}' has been killed.")
        return True
    
    return False

bz_plugins.register_callback(bz_plugins.event_type.CHAT, slash_command_handler)
