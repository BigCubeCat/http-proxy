import requests
import time
import random
import sys

PROXY = "http://proxy:8080"
URLS = [
    "http://www.gramota.ru/",
    "http://parallels.nsu.ru/WackoWiki/KursOperacionnyeSistemy/PraktikumPosixThreads/",
    "http://www.realtimerendering.com/blog/",  #
    "http://www.gutenberg.org/",
    "http://68k.news/",
    "http://rinkworks.com/",
    "http://ascii.textfiles.com/",
    "http://theoldnet.com/",
    "http://www.midwinter.com/",
    "http://static.kremlin.ru/media/events/video/ru/video_high/LJmJ5nrjhyCfVNDigS1CHdlmaG15G8cR.mp4",
    "http://kremlin.ru/"
]


def main():
    while True:
        url = random.choice(URLS)
        try:
            print(f"Requesting {url} via proxy...")
            response = requests.get(url,
                                    proxies={
                                        "http": PROXY,
                                        "https": PROXY
                                    })
            print(f"Response: {response.status_code}")
        except requests.exceptions.RequestException as e:
            print(f"Error: {e}")
        time.sleep(random.uniform(0.5, 2))  # Небольшая пауза между запросами


if __name__ == "__main__":
    main()
