import bzapi
from plugins.commands import Command, commands

c = Command("clientquery")
c.description = {
    "en": "Retrieve client version info from all users, or just the specified user if given.",
    "fr": "Récupérer les infos de version client de tous les joueurs ou d'un joueur donné."
}
c.handler = lambda tokens, from_id: (
    bzapi.send_chat_message(0, from_id, "Usage: /clientquery [player_name]")
    if len(tokens) > 2
    else (
        (lambda target_ids: (
            bzapi.send_chat_message(0, from_id, "No players found.")
            if not target_ids
            else bzapi.send_chat_message(
                0,
                from_id,
                "; ".join(f"{bzapi.get_player_name(pid)}: {bzapi.get_client_info(pid)}" for pid in target_ids)
            )
        ))(
            [pid] if len(tokens) == 2 and (pid := bzapi.get_player_by_name(tokens[1])) != 0
            else ([] if len(tokens) == 2 else bzapi.get_all_player_ids())
        )
        if len(tokens) == 1 or (len(tokens) == 2 and (pid := bzapi.get_player_by_name(tokens[1])) != 0)
        else bzapi.send_chat_message(0, from_id, f"Player '{tokens[1]}' not found.")
    )
)


c = Command("countdown")
c.description = {
    "en": "Start the shutdown countdown (seconds optional).",
    "fr": "Lancer le compte à rebours d'arrêt (secondes en option)."
}
c.handler = lambda tokens, from_id: (
    bzapi.send_chat_message(0, from_id, "Usage: /countdown [seconds]")
    if len(tokens) > 2 or (len(tokens) == 2 and not tokens[1].isdigit())
    else (
        bzapi.initiate_game_shutdown(int(tokens[1]) if len(tokens) == 2 else 10),
        bzapi.send_chat_message(0, from_id, f"Game shutdown countdown started ({int(tokens[1]) if len(tokens) == 2 else 10}s).")
    )
)

c = Command("date")
c.admin_only = False
c.description = {
    "en": "Respond with the current server date/time (same as /time).",
    "fr": "Répond avec la date/heure actuelle du serveur (comme /time)."
}
c.handler = lambda tokens, from_id: bzapi.send_chat_message(0, from_id, bzapi.get_server_datetime())


c = Command("flag")
c.description = {
    "en": "Give or take a flag namespace for a player (admin/testing).",
    "fr": "Donner ou retirer un drapeau (namespace) à un joueur (admin/tests)."
}
c.handler = lambda tokens, from_id: (
    bzapi.send_chat_message(0, from_id, "Usage: /flag <give|take> <player_name> <flag_namespace>")
    if len(tokens) != 4 or tokens[1].lower() not in ("give", "take")
    else (
        bzapi.send_chat_message(0, from_id, f"Player '{tokens[2]}' not found.")
        if (target_id := bzapi.get_player_by_name(tokens[2])) == 0
        else (
            (bzapi.give_player_flag(target_id, tokens[3]), bzapi.send_chat_message(0, from_id, f"Gave flag '{tokens[3]}' to '{tokens[2]}'."))
            if tokens[1].lower() == "give"
            else (bzapi.drop_player_flag(target_id), bzapi.send_chat_message(0, from_id, f"Removed flag from '{tokens[2]}'."))
        )
    )
)

c = Command("help")
c.admin_only = False
c.description = {
    "en": "Show usage information for a specified command.",
    "fr": "Afficher les informations d'utilisation pour une commande spécifiée."
}


def _help_handler(tokens, from_id):
    if len(tokens) != 2:
        bzapi.send_chat_message(0, from_id, "Usage: /help <command>")
        return

    target_cmd = tokens[1].lower()
    command = commands.get(target_cmd)
    if not command:
        bzapi.send_chat_message(0, from_id, f"Unknown command '{target_cmd}'.")
        return

    description = command.desc(bzapi.get_player_language(from_id))
    if description:
        bzapi.send_chat_message(0, from_id, description)
    command.show_usage(from_id)


c.handler = _help_handler


c = Command("me")
c.admin_only = False
c.description = {
    "en": 'Display an action as your callsign (e.g., "/me is hunting wabbits").',
    "fr": 'Affiche une action avec votre indicatif (ex. : "/me chasse des lapins").'
}
c.handler = lambda tokens, from_id: (
    bzapi.send_chat_message(0, from_id, 'Usage: /me <action>')
    if len(tokens) < 2
    else bzapi.send_chat_message(0, 0, f"{bzapi.get_player_name(from_id)} " + " ".join(tokens[1:]))
)


c = Command("msg")
c.admin_only = False
c.description = {
    "en": 'Send a private message to a player (e.g., "/msg Alice hello").',
    "fr": 'Envoyer un message privé à un joueur (ex. : "/msg Alice bonjour").'
}
c.handler = lambda tokens, from_id: (
    bzapi.send_chat_message(0, from_id, "Usage: /msg <player_name> <message>")
    if len(tokens) < 3
    else (
        bzapi.send_chat_message(0, from_id, f"Player '{tokens[1]}' not found.")
        if (target_id := bzapi.get_player_by_name(tokens[1])) == 0
        else bzapi.send_chat_message(from_id, target_id, " ".join(tokens[2:]))
    )
)


c = Command("quit")
c.admin_only = False
c.description = {
    "en": 'Leave the server with a goodbye message.',
    "fr": 'Quitter le serveur avec un message d\'au revoir.'
}
c.handler = lambda tokens, from_id: (
    (lambda msg: (bzapi.send_chat_message(0, 0, msg), bzapi.disconnect_player(from_id, msg)))(
        f"{bzapi.get_player_name(from_id)} has left the server." + ("" if len(tokens) == 1 else " " + " ".join(tokens[1:]))
    )
)

