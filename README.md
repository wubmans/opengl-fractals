export MESA_GL_VERSION_OVERRIDE=4.5
glxinfo | grep "OpenGL version"
glxinfo -B
echo 'export MESA_GL_VERSION_OVERRIDE=4.5' >> ~/.bashrc

 LIBGL_ALWAYS_SOFTWARE=true ./main