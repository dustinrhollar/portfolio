
void x11_opengl_begin_frame(r32 color[3])
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
    glClearColor(color[0], color[1], color[2], 1.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
}

void x11_opengl_end_frame(HostWnd *wnd)
{
    glXSwapBuffers(wnd->display, wnd->drawable);
}
