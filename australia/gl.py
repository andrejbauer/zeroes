import glfw
import math
import OpenGL.GL as gl
import cv2
import mediapipe as mp
import colorsys

def framebuffer_size_callback(window, width, height):
    gl.glViewport(0, 0, width, height)

def draw_gradient_disk(radius, hue):
    r, g, b = colorsys.hsv_to_rgb(hue, 1.0, 1.0)
    num_segments = 100
    gl.glBegin(gl.GL_TRIANGLE_FAN)
    gl.glColor4f(r, g, b, 1.0)  # Center color with full alpha
    gl.glVertex2f(0.0, 0.0)  # Center of the disk
    for i in range(num_segments + 1):
        theta = 2.0 * math.pi * i / num_segments
        x = radius * math.cos(theta)
        y = radius * math.sin(theta)
        gl.glColor4f(r, g, b, 0.0)  # Edge color with zero alpha
        gl.glVertex2f(x, y)
    gl.glEnd()

if not glfw.init():
    raise Exception("GLFW can't be initialized")

window = glfw.create_window(800, 800, "Interactive Disk with Body Posture Detection", None, None)
if not window:
    glfw.terminate()
    raise Exception("GLFW window can't be created")

glfw.make_context_current(window)
glfw.set_framebuffer_size_callback(window, framebuffer_size_callback)

gl.glMatrixMode(gl.GL_PROJECTION)
gl.glLoadIdentity()
gl.glOrtho(-1, 1, -1, 1, -1, 1)

# Enable blending
gl.glEnable(gl.GL_BLEND)
gl.glBlendFunc(gl.GL_SRC_ALPHA, gl.GL_ONE_MINUS_SRC_ALPHA)

mp_pose = mp.solutions.pose
pose = mp_pose.Pose()
cap = cv2.VideoCapture(0)

while not glfw.window_should_close(window):
    ret, frame = cap.read()
    if not ret:
        break

    frame_rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
    results = pose.process(frame_rgb)

    if results.pose_landmarks:
        height, width, _ = frame.shape
        landmarks = results.pose_landmarks.landmark

        right_wrist = landmarks[mp_pose.PoseLandmark.RIGHT_WRIST]

        radius = 0.1 + 0.4 * (1 - right_wrist.y)  # Y-coordinate controls the radius
        hue = right_wrist.x  # X-coordinate controls the hue

        gl.glClear(gl.GL_COLOR_BUFFER_BIT | gl.GL_DEPTH_BUFFER_BIT)
        draw_gradient_disk(radius, hue)

    glfw.swap_buffers(window)
    glfw.poll_events()

cap.release()
glfw.terminate()
