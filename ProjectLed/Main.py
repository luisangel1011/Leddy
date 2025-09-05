import sys
import asyncio


from PIL.ImageChops import overlay
from bleak import BleakClient, BleakScanner



from PyQt5.QtGui import QColor
from PyQt5.QtWidgets import (
    QApplication, QWidget, QVBoxLayout, QHBoxLayout,
    QLabel, QPushButton, QFrame, QSpacerItem, QSizePolicy, QColorDialog,
    QSystemTrayIcon, QMenu, QAction
)
from PyQt5.QtCore import Qt, QPoint, QRect,QPropertyAnimation,QSize
from PyQt5.QtGui import QIcon


from qasync import QEventLoop  # 🔹 Integración Qt + asyncio
from sympy.strategies.core import switch

CHARACTERISTIC_UUID = "abcd1234-5678-90ab-cdef-1234567890ab"


class FramelessResizableWindow(QWidget):
    def __init__(self):
        super().__init__()

        self.setWindowFlags(Qt.FramelessWindowHint | Qt.Window)
        self.resize(500, 10)
        self.move(0,0)
        self.setStyleSheet("background-color: #f4f6f9; ")

        layout = QVBoxLayout()
        layout.setContentsMargins(0, 0, 0, 0)

        # Crear ícono
        tray_icon = QSystemTrayIcon(QIcon("icono.ico"), parent=app)
        tray_icon.setToolTip("Mi aplicación en segundo plano")

        # Crear menú
        self.menu = QMenu()
        action_salir = QAction("Salir")
        action_salir.triggered.connect(app.quit)
        self.menu.addAction(action_salir)

        tray_icon.setContextMenu(self.menu)
        tray_icon.show()

        # 🔹 Header (zona arrastrable)
        self.header = QFrame()
        self.header.setFixedHeight(40)
        self.header.setStyleSheet("background-color: #0078d7;")

        header_layout = QHBoxLayout()
        header_layout.setContentsMargins(10, 0, 10, 0)

        # 🔹 Título
        self.title = QLabel("Project SCL")
        self.title.setStyleSheet("color: white; font-size: 14px; font-weight: bold;")
        header_layout.addWidget(self.title, alignment=Qt.AlignVCenter | Qt.AlignLeft)

        # 🔹 Espaciador
        header_layout.addItem(QSpacerItem(40, 20, QSizePolicy.Expanding, QSizePolicy.Minimum))


        self.header.setLayout(header_layout)
        layout.addWidget(self.header)

        # 🔹 Contenido
        self.body = QFrame()
        self.body.setStyleSheet("background:#ffffff;")
        self.body_layout = QVBoxLayout(self.body)

        # Div del diseño
        DivDiseño = QFrame()
        DivDiseño.setObjectName("DivDiseño")
        DivDiseño.setStyleSheet("background:#828282")

        DivDiseñoLayout = QVBoxLayout(DivDiseño)

        lblTitle2 = QLabel("Selector de color")
        lblTitle2.setSizePolicy(QSizePolicy.Preferred, QSizePolicy.Fixed)
        lblTitle2.setAlignment(Qt.AlignHCenter)
        lblTitle2.setFixedHeight(30)
        lblTitle2.setStyleSheet("background:white; font-size:20px")
        DivDiseñoLayout.addWidget(lblTitle2)

        btnColor = QPushButton("color Solido")
        btnColor.clicked.connect(lambda: self.Funciones("solido"))
        DivDiseñoLayout.addWidget(btnColor)

        btnArcoiris = QPushButton("Efecto Arcoiris")
        btnArcoiris.clicked.connect(lambda: self.Funciones("arcoiris"))
        DivDiseñoLayout.addWidget(btnArcoiris)

        btnDesvanecer = QPushButton("Efecto Desvanecer")
        btnDesvanecer.clicked.connect(lambda: self.Funciones("desvanecer"))
        DivDiseñoLayout.addWidget(btnDesvanecer)


        self.body_layout.addWidget(DivDiseño)
        layout.addWidget(self.body)

        self.setLayout(layout)
    #Ventanas emergentes
        self.overlay = QFrame(self)
        self.overlay.setGeometry(100, 100, 200, 100)
        self.overlay.setStyleSheet("background-color: rgba(0, 0, 0, 180); border-radius: 8px;")

        # Layout interno del overlay
        overlay_layout = QVBoxLayout(self.overlay)
        self.lblOverlay = QLabel("Buscando dispositivo...", self.overlay)
        self.lblOverlay.setStyleSheet("color: white; font-size: 14px;")
        self.lblOverlay.setAlignment(Qt.AlignCenter)
        overlay_layout.addWidget(self.lblOverlay)

        self.overlay.setHidden(True)  # al inicio oculto
        self.body.setHidden(True)
        self.header.setFixedHeight(10)

        # Variables de arrastre/redimensión
        self.dragging = False
        self.offset = QPoint()
        self.resizing = False
        self.drag_start = None
        self.start_geom = None
        self.resize_margin = 8
        self.resize_directions = {"left": False, "right": False, "top": False, "bottom": False}
        self.maximized = False

    def Funciones(self, funcion):
        match funcion:
            case "solido":
                color = QColorDialog.getColor(QColor("#ffffff"), self, "Selecciona un color")
                if color.isValid():
                    # 🔹 Llamamos al BLE en asyncio sin bloquear Qt
                    asyncio.create_task(self.CambiarColor(color.red(), color.green(),color.blue(), "solido"))
            case "arcoiris":
                asyncio.create_task(self.CambiarColor("", "", "", "arcoiris"))
            case "desvanecer":
                asyncio.create_task(self.CambiarColor("", "", "", "desvanecer"))


    async def CambiarColor(self, R, G, B,funcion):
        esp32 = None

        while not esp32:
            print("Buscando dispositivos BLE...")
            self.overlay.setHidden(False)
            devices = await BleakScanner.discover()

            for d in devices:
                try:
                    if d.name and "ESP32C3_BT" in d.name:
                        esp32 = d
                        break
                except Exception as e:

                    print("Error leyendo dispositivo:", e)

            if not esp32:
                self.lblOverlay.setText("Dispositivo no encontrado, re intentando en 3s")
                print("ESP32-C3 no encontrado, reintentando en 3s...")
                await asyncio.sleep(3)
                self.lblOverlay.setText("Buscando dispositivo...")

        self.lblOverlay.setText(f"Conectando a {esp32.name} ({esp32.address})...")
        print(f"Conectando a {esp32.name} ({esp32.address})...")


        try:
            async with BleakClient(esp32.address) as client:
                print("Conectado:", client.is_connected)
                self.lblOverlay.setText(f"Conectado:{client.is_connected}")
                self.overlay.setHidden(True)
                match funcion:
                    case "solido":
                        payload = f"{R},{B},{G},{funcion}".encode()
                        await client.write_gatt_char(CHARACTERISTIC_UUID, payload)
                        print(f"LEDs cambiados a RGB({R},{G},{B})")
                    case "arcoiris":
                        payload = f"{funcion}".encode()
                        await client.write_gatt_char(CHARACTERISTIC_UUID, payload)
                        print(f"LEDs con efecto arcoiris")
                    case "desvanecer":
                        payload = f"{R},{B},{G},{funcion}".encode()
                        await client.write_gatt_char(CHARACTERISTIC_UUID, payload)
                        print(f"LEDs cambiados a RGB({R},{G},{B})")

        except Exception as e:
            print("Error en la conexión BLE:", e)
            self.lblOverlay.setText(f"Conectado:{client.is_connected}")
            self.overlay.setHidden(True)

    def OcultarTodo(self):
        self.body.setHidden(True)
        self.header.setFixedHeight(10)
        # 🔹 Animar altura de la ventana
        self.anim_window = QPropertyAnimation(self, b"size")
        self.anim_window.setDuration(200)
        self.anim_window.setStartValue(QSize(500, 400))  # QRect actual
        self.anim_window.setEndValue(QSize(500, 10))  # reducimos altura
        self.anim_window.start()


    def MostrarTodo(self):
        self.body.setHidden(False)

        # 🔹 Animar altura de la ventana
        self.anim_window = QPropertyAnimation(self, b"size")
        self.anim_window.setDuration(200)
        self.anim_window.setStartValue(QSize(500, 10))  # QRect actual
        self.anim_window.setEndValue(QSize(500, 400))  # reducimos altura
        self.anim_window.start()
        self.header.setFixedHeight(40)

    def enterEvent(self, event):
        tam = self.size().height()
        if (tam <= 10):
            self.MostrarTodo()

    def leaveEvent(self, event):
        tam = self.size().height()
        if (self.size().height() > 10):
            self.OcultarTodo()

    # 🔹 Estilos de botones
    def button_style(self, color):
        if color == "red":
            return """
                QPushButton {
                    background-color: red; color: white; border: none;
                    border-radius: 4px; font-weight: bold;
                }
                QPushButton:hover { background-color: darkred; }
            """
        else:  # gris
            return """
                QPushButton {
                    background-color: #444; color: white; border: none;
                    border-radius: 4px; font-weight: bold;
                }
                QPushButton:hover { background-color: #666; }
            """






# 🔹 Ejecutar Qt y asyncio juntos
app = QApplication(sys.argv)
loop = QEventLoop(app)
asyncio.set_event_loop(loop)

window = FramelessResizableWindow()
window.show()

with loop:
    loop.run_forever()
