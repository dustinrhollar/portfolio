#ifndef SPLICER_GAME_CAMERA_H
#define SPLICER_GAME_CAMERA_H

enum CameraMovement
{
    CAMERA_FORWARD,
    CAMERA_BACKWARD,
    CAMERA_LEFT,
    CAMERA_RIGHT,
    CAMERA_ARROW_UP,
    CAMERA_ARROW_DOWN,
    CAMERA_ARROW_RIGHT,
    CAMERA_ARROW_LEFT,
};

const r32 YAW         = -90.0f;
const r32 PITCH       = 0.0f;
const r32 SPEED       = 102.5f;
const r32 SENSITIVITY = 0.1f;
const r32 ZOOM        = 45.0f;

class Camera {
    public:
    
    // Positional vectors
    Vec3 Position;
    Vec3 Front;
    Vec3 Up;
    Vec3 Right;
    Vec3 WorldUp;
    
    // Mouse Position
    r32 MouseXPos;
    r32 MouseYPos;
    
    // Camera options
    r32 MovementSpeed;
    r32 MouseSensitivity;
    r32 Zoom;
    
    Camera(Vec3 position,
           Vec3 forward,
           Vec3 up)
    {
        Position = position;
        WorldUp  = up;
        
        MovementSpeed    = SPEED;
        MouseSensitivity = SENSITIVITY;
        Zoom             = ZOOM;
        
        MouseXPos = 0.0f;
        MouseYPos = 0.0f;
        
        Front = norm(forward);
        Up    = norm(up);
        Right = cross(up, forward);
    }
    
    Mat4 GetViewMatrix()
    {
        return LookAt(Position, Position + Front, Up);
    }
    
    virtual void ProcessKeyboardInput(CameraMovement direction, float delta_time) = 0;
    
    protected:
    
    void RotateX(float angle) {
        Vec3 haxis = norm(cross(WorldUp, Front));
        
        Quaternion result = CreateQuaternion(haxis, angle);
        Quaternion conj   = conjugate(result);
        
        Quaternion fq = {};
        fq.x = Front.x;
        fq.y = Front.y;
        fq.z = Front.z;
        fq.w = 0;
        
        result = QuaternionMul(QuaternionMul(result, fq), conj);
        
        Front = norm(result.xyz);
    }
    
    void RotateY(float angle) {
        Vec3 haxis = norm(cross(WorldUp, Front));
        
        Quaternion result = CreateQuaternion(WorldUp, angle);
        Quaternion conj   = conjugate(result);
        
        Quaternion fq = {};
        fq.x = Front.x;
        fq.y = Front.y;
        fq.z = Front.z;
        fq.w = 0;
        
        result = QuaternionMul(QuaternionMul(result, fq), conj);
        
        Front = norm(result.xyz);
    }
    
    void UpdateCameraVectors()
    {
        Right = norm(cross(Front, WorldUp));
        Up    = norm(cross(Right, Front));
    }
};

class PlayerCamera : public Camera
{
    public:
    
    PlayerCamera(Vec3 position = {0.0f,0.0f,0.0f},
                 Vec3 forward  = {0.0f,0.0f,1.0f},
                 Vec3 up       = {0.0f,1.0f,0.0f})
        : Camera(position, forward, up)
    {
    }
    
    virtual void ProcessKeyboardInput(CameraMovement direction, float delta_time) override
    {
        float velocity = MovementSpeed    * delta_time;
        float rotation = MouseSensitivity * delta_time;
        
        switch (direction)
        {
            case CAMERA_FORWARD:
            {
                Position += Front * velocity;
            } break;
            case CAMERA_BACKWARD:
            {
                Position -= Front * velocity;
            } break;
            case CAMERA_LEFT:
            {
                Position -= Right * velocity;
            } break;
            case CAMERA_RIGHT:
            {
                Position += Right * velocity;
            } break;
        }
        UpdateCameraVectors();
    }
    
    void ProcessMouseInput(r32 xpos, r32 ypos, r32 width, r32 height,
                           bool constrain_pitch = true)
    {
        r32 half_width  = width / 2.0f;
        r32 half_height = height / 2.0f;
        
        if (xpos != MouseXPos || ypos != MouseYPos) {
            r32 xoffset = (xpos - MouseXPos);
            r32 yoffset = (ypos - MouseYPos);
            
            MouseXPos = xpos;
            MouseYPos = ypos;
            
            xoffset *= MouseSensitivity;
            yoffset *= MouseSensitivity;
            
            Vec2 mouse_rotation = {xoffset, yoffset};
            
            RotateX(mouse_rotation.y);
            RotateY(-mouse_rotation.x);
            UpdateCameraVectors();
        }
    }
};

class DevCamera : public Camera
{
    DevCamera(Vec3 position = {0.0f,0.0f,0.0f},
              Vec3 forward  = {0.0f,0.0f,1.0f},
              Vec3 up       = {0.0f,1.0f,0.0f})
        : Camera(position, forward, up)
    {
    }
    
    virtual void ProcessKeyboardInput(CameraMovement direction, float delta_time) override
    {
        float velocity = MovementSpeed    * delta_time;
        float rotation = MouseSensitivity * delta_time;
        
        switch (direction)
        {
            case CAMERA_FORWARD:
            {
                Position += Front * velocity;
            } break;
            case CAMERA_BACKWARD:
            {
                Position -= Front * velocity;
            } break;
            case CAMERA_LEFT:
            {
                Position -= Right * velocity;
            } break;
            case CAMERA_RIGHT:
            {
                Position += Right * velocity;
            } break;
            case CAMERA_ARROW_UP:
            {
                RotateX(-rotation);
            } break;
            case CAMERA_ARROW_DOWN:
            {
                RotateX(rotation);
            } break;
            case CAMERA_ARROW_RIGHT:
            {
                RotateY(-rotation);
            } break;
            case CAMERA_ARROW_LEFT:
            {
                RotateY(rotation);
            } break;
            default: break;
        }
        
        UpdateCameraVectors();
    }
};

#endif //SPLICER_GAME_CAMERA_H
