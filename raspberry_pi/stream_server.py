from http.server import BaseHTTPRequestHandler, HTTPServer
import threading
import cv2

# holds the most recent annotated frame to be served to connected clients
annotated_frame = None
# lock protecting annotated_frame since it is written by the camera thread and read by the HTTP server thread
annotated_lock = threading.Lock()

# HTTP request handler that serves a stream on GET /stream.
# each response part is a JPEG-encoded frame, allowing browsers to display a live feed
class StreamHandler(BaseHTTPRequestHandler):
    def log_message(self, *args): pass

    def do_GET(self):
        if self.path == '/stream':
            # multipart/x-mixed-replace tells the browser to replace each part in place,
            # producing a continuous video effect without requiring JavaScript
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

# called by the camera thread to update the frame that will be served to stream clients
def set_frame(frame):
    global annotated_frame
    with annotated_lock:
        annotated_frame = frame

# starts the HTTP server on port 8080 in a background daemon thread so it does not block the main loop
def start():
    threading.Thread(
        target=lambda: HTTPServer(('0.0.0.0', 8080), StreamHandler).serve_forever(),
        daemon=True
    ).start()