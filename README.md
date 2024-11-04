# 🌐 ESP32 Google PubSub

Welcome to the **ESP32 Google PubSub** project! 🎉 This repository is a continuation of the [ESP32 JWT Auth](https://github.com/euginfrancis/ESP32_JWT_Auth) project, where we integrate Google Pub/Sub with the ESP32 microcontroller for efficient messaging and event-driven architectures.

## 📜 Overview

This project demonstrates how to use Google Cloud Pub/Sub with the ESP32 microcontroller, allowing for seamless communication between IoT devices and cloud services. ☁️ It leverages JSON Web Tokens (JWT) for secure authentication and authorization. 🔒

## ✨ Features

- **Secure Communication**: Utilize JWT for authentication with Google Cloud services. 🔐
- **Real-time Messaging**: Publish and subscribe to messages in real-time using Google Pub/Sub. 📬
- **IoT Integration**: Connect your ESP32 devices to the cloud for scalable IoT solutions. 🌍

## ⚙️ Prerequisites

Before you begin, ensure you have met the following requirements:

- An **ESP32** development board. 🛠️
- A Google Cloud account with billing enabled. 💳
- Google Cloud SDK installed (for setting up Pub/Sub). 📦
- Basic knowledge of Arduino IDE or PlatformIO. 📚

## 🚀 Installation

1. Clone this repository:

   ```bash
   git clone https://github.com/euginfrancis/ESP32_Google_PubSub.git
   cd ESP32_Google_PubSub

### Set up your Google Cloud Pub/Sub:

1. Create a new project on Google Cloud Console. 🖥️
2. Enable the Pub/Sub API. ⚡
3. Create a service account and download the JSON key file. 📄
4. Update the code with your credentials and project settings. 🔧

## 📬 Usage

### 📤 Publishing Messages

To publish messages to a topic, ensure you configure the following in your code:
```cpp
const char PRIVATE_KEY[] = "Your private key";
const char* projectId = "Your pubsub project id";
const char* topicName = "Your pubsub topic";
const char* subscription_id = "Your pubsub subscription id";
//Please ensure the private key is formatted correctly.
```
## 🤝 Contributing

Contributions are welcome! Please fork the repository and submit a pull request for any improvements or new features. 💡

## 📜 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details. 📃

## 🙏 Acknowledgments

- [Google Cloud Pub/Sub Documentation](https://cloud.google.com/pubsub/docs) 📚
- [ESP32 Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html) 🛠️


