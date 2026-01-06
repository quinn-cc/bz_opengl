import bzapi
from plugins.commands import Command, commands

c = Command("kick")
c.description = {
    "en": "Kick a named player off the server.",
    "fr": "Expulser un joueur nommé du serveur."
}
c.handler = lambda tokens, from_id: (
    bzapi.send_chat_message(0, from_id, "Usage: /kick <player_name>")
    if len(tokens) != 2
    else (
        bzapi.send_chat_message(0, from_id, f"Player '{tokens[1]}' not found.")
        if (target_id := bzapi.get_player_by_name(tokens[1])) == 0
        else (
            bzapi.kick_player(target_id, "Kicked by server"),
            bzapi.send_chat_message(0, from_id, f"Kicked '{tokens[1]}'.")
        )
    )
)


c = Command("kill")
c.description = {
    "en": "Kill a player (shows as destroyed by the server).",
    "fr": "Tuer un joueur (affiché comme détruit par le serveur)."
}
c.handler = lambda tokens, from_id: (
    bzapi.send_chat_message(0, from_id, "Usage: /kill <player_name>")
    if len(tokens) != 2
    else (
        bzapi.send_chat_message(0, from_id, f"Player '{tokens[1]}' not found.")
        if (target_id := bzapi.get_player_by_name(tokens[1])) == 0
        else (
            bzapi.kill_player(target_id),
            bzapi.send_chat_message(0, from_id, f"Killed '{tokens[1]}'.")
        )
    )
)

c = Command("reset")
c.description = {
    "en": "Reset a server variable to its default setting.",
    "fr": "Réinitialiser un paramètre serveur à sa valeur par défaut."
}
c.handler = lambda tokens, from_id: (
    bzapi.send_chat_message(0, from_id, "Usage: /reset <setting>")
    if len(tokens) != 2
    else (
        bzapi.reset_parameter(tokens[1]),
        bzapi.send_chat_message(0, from_id, f"World setting '{tokens[1]}' reset to default.")
    )
)

c = Command("set")
c.description = {
    "en": "Set a world setting to a numeric value.",
    "fr": "Définir un paramètre du monde à une valeur numérique."
}
c.handler = lambda tokens, from_id: (
    bzapi.send_chat_message(0, from_id, "Usage: /set <setting> <value>")
    if len(tokens) != 3
    else (
        bzapi.send_chat_message(0, from_id, "Value must be a number.")
        if not (lambda: (float(tokens[2]), True)[1])()
        else (
            bzapi.set_parameter(tokens[1], float(tokens[2])),
            bzapi.send_chat_message(0, from_id, f"World setting '{tokens[1]}' set to {float(tokens[2])}.")
        )
    )
)



c = Command("superkick")
c.description = {
    "en": "Kick all players on a team, or everyone if no team is given.",
    "fr": "Expulser tous les joueurs d'une équipe, ou tout le monde si aucune équipe n'est donnée."
}
c.handler = lambda tokens, from_id: (
    bzapi.send_chat_message(0, from_id, "Usage: /superkick [team_id]")
    if len(tokens) > 2 or (len(tokens) == 2 and not tokens[1].isdigit())
    else (
        (lambda team_filter: (
            (lambda ids: (
                bzapi.send_chat_message(0, from_id, "No players found to kick.") if not ids else (
                    [bzapi.kick_player(pid, f"Kicked by server{' (team ' + str(team_filter) + ')' if team_filter is not None else ''}") for pid in ids],
                    bzapi.send_chat_message(0, from_id, f"Kicked {len(ids)} player(s).")
                )
            ))([pid for pid in bzapi.get_all_player_ids() if team_filter is None or bzapi.get_player_team(pid) == team_filter])
        ))(None if len(tokens) == 1 else int(tokens[1]))
    )
)


c = Command("superkill")
c.description = {
    "en": "Kill all players on a team, or everyone if no team is given.",
    "fr": "Tuer tous les joueurs d'une équipe, ou tout le monde si aucune équipe n'est donnée."
}
c.handler = lambda tokens, from_id: (
    bzapi.send_chat_message(0, from_id, "Usage: /superkill [team_id]")
    if len(tokens) > 2 or (len(tokens) == 2 and not tokens[1].isdigit())
    else (
        (lambda team_filter: (
            (lambda ids: (
                bzapi.send_chat_message(0, from_id, "No players found to kill.") if not ids else (
                    [bzapi.kill_player(pid) for pid in ids],
                    bzapi.send_chat_message(0, from_id, f"Killed {len(ids)} player(s).")
                )
            ))([pid for pid in bzapi.get_all_player_ids() if team_filter is None or bzapi.get_player_team(pid) == team_filter])
        ))(None if len(tokens) == 1 else int(tokens[1]))
    )
)

