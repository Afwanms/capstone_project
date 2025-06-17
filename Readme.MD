# Wearable Device untuk Mendeteksi Postur Tubuh saat Mengangkat Beban
![Body Posture](asset/img/Body_Posture.jpeg)

K2A - Kelompok 3
Anggota : 
- Zenitha Shaula Lora   (225150301111034)
- Audrey Zakiya Trustee (225150307111003)	
- Adinni Salsabillah 	(225150307111014)
- Afwan Maulana Sidqi 	(225150307111033)
- Alfi Hisan Usri 		(225150307111048)

## Deskripsi Proyek
Wearable device ini melakukan deteksi postur tubuh ketika sedang mengangkat beban berdasarkan gerakan berupa akselerasi dan kecepatan sudut. Postur tubuh dideteksi menggunakan algoritma 1D-CNN dengan kelas tegak, bungkuk, dan miring. Sistem memberikan peringatan berupa getaran jika postur tubuh sedang berada dalam posisi miring dan bungkuk. Data dan posisi tubuh saat ini akan ditampilkan dalam sebuah dashboard.

## Alat yang Digunakan
- Sensor MPU6050 : Menangkap data gerakan berupa akselerasi dan kecepatan sudut
- ESP32 : Mikrokontroler yang mengontrol seluruh sistem
- Coin Vibration : Aktuator berupa getaran
- Switch : Menyalakan/mematikan perangkat
- Baterai : Baterai 3.7V 2800mAh
- TensorFlow Lite : Mengkompres model agar kompatibel di ESP32
- MQTT : Protokol komunikasi menggunakan mosquitto sebagai broker
- Flask API : Framework dashboard

## Arsitektur Sistem

### Berikut merupakan arsitektur dari sistem ini:

### 1. Blok Diagram
![Gambar Blok Diagram](asset/img/blok_diagram.png)
### 2. Wiring Diagram
![Gambar Wiring Diagram](asset/img/wiring_diagram.png)
### 3. Perangkat Wearable
![Gambar Perangkat Wearable](asset/img/wearable_device.jpg)

## Algoritma yang Digunakan
### 1D-CNN
Model 1D-CNN telah dilatih sebelumnya untuk mengenali pola gerakan berdasarkan urutan data dari sensor MPU6050. Model ini diimplementasikan dalam ESP32 dan digunakan untuk mengklasifikasikan postur tubuh pengguna ke dalam kategori yang telah ditentukan (tegak, bungkuk, miring).

## Dashboard

## Demo
<video controls src="asset/img/Demo_Alat.mp4" title="Demo Alat"></video>