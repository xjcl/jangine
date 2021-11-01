
# auto-restart every 10 minutes because i leak memory haha
while true; do
    timeout 600 python lichess-bot.py
done

