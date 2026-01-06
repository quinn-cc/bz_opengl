import bzapi

PREFIX = "/"
commands = {}

class Command:
    def __init__(self, name: str, activate: bool = True):
        self.name = name
        self.handler = None
        self.admin_only = True
        self.description = {}
        self.aliases = []
        self.usage = ""
        if activate:
            commands[self.name] = self

    def append_usage(self, usage: str) -> None:
        self.usage += f" {usage}"

    def show_usage(self, to_player_id: int) -> None:
        USAGE = {
            "en": "Usage",
            "fr": "Utilisation",
            "de": "Verwendung",
            "es": "Uso",
            "pt": "Uso",
            "ja": "使い方",
            "ru": "Использование",
            "zh": "用法"
        }
        #usage = USAGE.get(bzapi.get_player_language(to_player_id), "Usage")
        usage = USAGE.get("en", "Usage")
        usage += ": " + PREFIX + self.name
        if self.usage:
            usage += self.usage
        bzapi.send_chat_message(0, to_player_id, usage)

    def show_info(self, to_player_id: int, info: str) -> None:
        bzapi.send_chat_message(0, to_player_id, info)

    def desc(self, lang="en"):
        if lang in self.description:
            return self.description[lang]
        return next(iter(self.description.values()), "")



def handler(from_id: int, to_id: int, text: str):
    if not text or not text.startswith(PREFIX):
        return False
    tokens = text[len(PREFIX):].split()
    if not tokens:
        return False
    cmd = tokens[0].lower()
    command = commands.get(cmd)
    if not command or not command.handler:
        return False
    #if command.admin_only and not bzapi.is_player_admin(from_id):
    #    bzapi.send_chat_message(0, from_id, "Admin only command.")
    #    return True
    command.handler(tokens, from_id)
    return True

bzapi.register_callback(bzapi.event_type.CHAT, handler)


c = Command("?")
c.admin_only = False
c.description = {
    "en": "List all available commands.",
    "fr": "Lister toutes les commandes disponibles.",
    "de": "Alle verfügbaren Befehle auflisten.",
    "es": "Listar todos los comandos disponibles.",
    "pt": "Listar todos os comandos disponíveis.",
    "ja": "利用可能なすべてのコマンドを一覧表示します。",
    "ru": "Показать все доступные команды.",
    "zh": "列出所有可用命令。",
}
def _list_commands_handler(tokens, from_id):
    #lang = bzapi.get_player_language(from_id)
    lang = "en"
    entries = []
    for name, command in sorted(commands.items()):
        description = command.desc(lang).strip()
        if description:
            entries.append(f"{name} - {description}")
            #entries.append(f"{name} -  [description]")
        else:
            entries.append(name)
    msg = "Commands:\n" + "\n".join(entries)
    #print(msg)
    bzapi.send_chat_message(0, from_id, msg)


c.handler = _list_commands_handler
