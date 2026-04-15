from http.server import BaseHTTPRequestHandler, HTTPServer
import threading
import cv2

annotated_frame = None
annotated_lock = threading.Lock()

class StreamHandler(BaseHTTPRequestHandler):
    def log_message(self, *args): pass

    def do_GET(self):
        if self.path == '/stream':
            self.send_response(200)
            self.send_header('Content-type', 'multipart/x-mixed-replace; boundary=frame')
            self.end_headers()
            while True:
                with annotated_lock:
                    if annotated_frame is None:
                        continue
                    _, jpeg = cv2.imencode('.jpg', annotated_frame)
                data = jpeg.tobytes()
                self.wfile.write(b'--frame\r\nContent-Type: image/jpeg\r\n\r\n' + data + b'\r\n')
        else:
            self.send_response(404)
            self.end_headers()

def set_frame(frame):
    global annotated_frame
    with annotated_lock:
        annotated_frame = frame

def start():
    threading.Thread(
        target=lambda: HTTPServer(('0.0.0.0', 8080), StreamHandler).serve_forever(),
        daemon=True
    ).start()