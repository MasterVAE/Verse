import socket

def send_data_tcp(host='ru.tuna.am', port=32585):
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

            message = input()

            if isinstance(message, str):
                message = message.encode('utf-8')
   
            client_socket.sendall(message)
            print(f"Отправлено {len(message)} байт")

            response = client_socket.recv(1024)
            if response:
                print(f"Получен ответ: {response.decode('utf-8')}")

        
    except ConnectionRefusedError:
        print("Соединение отклонено. Убедитесь, что сервер запущен.")
    except Exception as e:
        print(f"Произошла ошибка: {e}")

if __name__ == "__main__":
    send_data_tcp()