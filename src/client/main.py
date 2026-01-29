import socket

def send_data_tcp(host='ru.tuna.am', port=37710):
    """
    Функция для отправки данных по TCP соединению
    """
    try:
        # Создаем TCP сокет
        client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        
        # Подключаемся к серверу
        print(f"Подключаюсь к {host}:{port}...")
        client_socket.connect((host, port))
        print("Соединение установлено!")
        
        while True:

            response = client_socket.recv(1024)
            if response:
                print(f"Получен ответ: {response.decode('utf-8')}")

            # Отправляем данные (не забудьте преобразовать строку в байты)
            message = input()
            message = str(len(message)) + " " + message

            if isinstance(message, str):
                message = message.encode('utf-8')

   

            client_socket.sendall(message)
            print(f"Отправлено {len(message)} байт")

        
    except ConnectionRefusedError:
        print("Соединение отклонено. Убедитесь, что сервер запущен.")
    except Exception as e:
        print(f"Произошла ошибка: {e}")

# Использование
if __name__ == "__main__":
    # Пример 1: Простая отправка
    send_data_tcp('127.0.0.1', 8080)
    
    # Пример 2: Отправка нескольких сообщений
    # send_data_tcp('example.com', 80, 'GET / HTTP/1.1\r\nHost: example.com\r\n\r\n')