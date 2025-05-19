import os
import logging
import psycopg2
import pytz
from datetime import datetime, timedelta
from psycopg2.extras import RealDictCursor
from telegram import Update, ReplyKeyboardMarkup, KeyboardButton
from telegram.ext import (
    Application,
    CommandHandler,
    MessageHandler,
    ContextTypes,
    ConversationHandler,
    filters,
)

# constants and config
DB_PARAMS = {
    "host": os.environ["POSTGRES_HOST"],
    "port": os.environ["POSTGRES_PORT"],
    "user": os.environ["POSTGRES_USER"],
    "password": os.environ["POSTGRES_PASSWORD"],
    "dbname": os.environ["POSTGRES_DB"],
}
TOKEN = os.environ["TELEGRAM_TOKEN"]
ASK_NICK = 1
MOSCOW_TZ = pytz.timezone("Europe/Moscow")

notified_ids = set()


# /start
async def start(update: Update, context: ContextTypes.DEFAULT_TYPE):
    keyboard = [[KeyboardButton("🔔 Подключить уведомления")]]
    reply_markup = ReplyKeyboardMarkup(keyboard, resize_keyboard=True)
    await update.message.reply_text(
        "👋 Привет! Это телграм-бот нашей Pro100 Drift School 😎🏎️! \n\n"
        "Он будет напоминать тебе о грядущих занятиях и твоих новых зап."
        "Нажми кнопку ниже чтобы подключить уведомления 👇",
        reply_markup=reply_markup
    )

# button click
async def ask_nick(update: Update, context: ContextTypes.DEFAULT_TYPE):
    await update.message.reply_text("✍ Введи свой никнейм (как при регистрации):")
    return ASK_NICK

# get and add nickname
async def handle_nick(update: Update, context: ContextTypes.DEFAULT_TYPE):
    nickname = update.message.text.strip()
    telegram_id = update.effective_user.id

    try:
        with psycopg2.connect(**DB_PARAMS) as conn:
            with conn.cursor() as cur:
                cur.execute("SELECT telegram_id FROM users WHERE nickname = %s", (nickname,))
                result = cur.fetchone()

                if result is None:
                    await update.message.reply_text("❌ Пользователь не найден.")
                    return ASK_NICK

                if result[0] is not None and int(result[0]) != telegram_id:
                    await update.message.reply_text("🔐 Этот ник уже привязан.")
                    return ConversationHandler.END

                cur.execute("UPDATE users SET telegram_id = %s WHERE nickname = %s", (telegram_id, nickname))
                await update.message.reply_text("✅ Уведомления подключены!")
                return ConversationHandler.END

    except Exception as e:
        logging.error(f"DB error: {e}")
        await update.message.reply_text("❌ Ошибка базы данных.")
        return ConversationHandler.END

# task for notifications
async def notify_task(context: ContextTypes.DEFAULT_TYPE):
    bot = context.bot
    now = datetime.now(MOSCOW_TZ)
    target_from = now + timedelta(minutes=59)
    target_to = now + timedelta(minutes=61)

    try:
        with psycopg2.connect(**DB_PARAMS, cursor_factory=RealDictCursor) as conn:
            with conn.cursor() as cur:
                cur.execute("""
                    SELECT u.telegram_id, u.first_name, s.available_date, s.start_time
                    FROM schedule s
                    JOIN users u ON s.user_id = u.user_id
                    WHERE s.is_booked = true
                    AND u.telegram_id IS NOT NULL
                """)
                rows = cur.fetchall()

        notified = False
        for row in rows:
            schedule_dt = datetime.combine(row["available_date"], row["start_time"])
            schedule_dt = MOSCOW_TZ.localize(schedule_dt)

            if target_from <= schedule_dt <= target_to:
                msg = f"⚠ Привет, {row['first_name']}! Через час у тебя занятие."
                await bot.send_message(chat_id=row["telegram_id"], text=msg)
                notified = True

        if not notified:
            logging.info("ℹ Нет запланированных уведомлений на текущий час.")

    except Exception as e:
        logging.error(f"❌ Ошибка в notify_task: {e}")

async def notify_new_bookings(context: ContextTypes.DEFAULT_TYPE):
    try:
        with psycopg2.connect(**DB_PARAMS, cursor_factory=RealDictCursor) as conn:
            with conn.cursor() as cur:
                cur.execute("""
                    SELECT s.schedule_id, u.telegram_id, u.first_name, s.available_date, s.start_time
                    FROM schedule s
                    JOIN users u ON s.user_id = u.user_id
                    WHERE s.is_booked = TRUE AND u.telegram_id IS NOT NULL
                """)
                rows = cur.fetchall()

                for row in rows:
                    if row["schedule_id"] in notified_ids:
                        continue

                    msg = f"✅ {row['first_name']}, вы записаны на занятие {row['available_date'].strftime('%d.%m.%Y')} в {row['start_time'].strftime('%H:%M')}!"
                    await context.bot.send_message(chat_id=row["telegram_id"], text=msg)

                    notified_ids.add(row["schedule_id"])

    except Exception as e:
        logging.error(f"[notify_new_bookings] Ошибка: {e}")



# main launch
def main():
    logging.basicConfig(level=logging.INFO)
    app = Application.builder().token(TOKEN).build()

    # command handler "Подключить уведомления"
    conv = ConversationHandler(
        entry_points=[MessageHandler(filters.Regex("^🔔 Подключить уведомления$"), ask_nick)],
        states={ASK_NICK: [MessageHandler(filters.TEXT & ~filters.COMMAND, handle_nick)]},
        fallbacks=[],
    )

    app.add_handler(CommandHandler("start", start))
    app.add_handler(conv)

    # task scheduler
    app.job_queue.run_repeating(notify_task, interval=60)
    app.job_queue.run_repeating(notify_new_bookings, interval=30)

    app.run_polling()

if __name__ == "__main__":
    main()
