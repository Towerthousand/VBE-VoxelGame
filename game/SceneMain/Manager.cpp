#include "Manager.hpp"

Manager<RenderTargetBase> FrameBuffers = Manager<RenderTargetBase>();
Manager<Texture2D> Textures2D = Manager<Texture2D>();
Manager<MeshBase> Meshes = Manager<MeshBase>();
Manager<ShaderProgram> Programs = Manager<ShaderProgram>();
Manager<Texture3D> Textures3D = Manager<Texture3D>();
Manager<Texture2DArray> Textures2DArrays = Manager<Texture2DArray>();
