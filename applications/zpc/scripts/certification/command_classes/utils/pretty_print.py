from colorist import Color, BgColor, green, red, blue, yellow, magenta

def welcome_message(msg):
    print(f"\U0001F920 {BgColor.GREEN}{msg}{BgColor.OFF} \U0001F920")

def check_success(msg):
    green(f"\U00002705 {msg}")

def check_fail(msg):
    red(f"\U0000274C {msg}")

def action_success(msg):
    green(msg)

def action_failed(msg):
    red(msg)

def action_warning(msg):
    yellow(msg)

def action_description(msg):
    magenta(f"\U000025B6  {msg}")

def action_step(msg):
    print(msg)

def publish_topic(msg):
    blue(f"\U000027A1  Publish : {msg}") 
    pause()

def pause():
    input("\U00002328  Press Enter to publish topic...") 