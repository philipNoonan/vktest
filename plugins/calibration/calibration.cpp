#include "vulkanexamplebase.h"

struct Vertex {
	float pos[3];
	float uv[2];
};

class HvsPlugin : public VulkanExampleBase
{
private:
	vks::Texture2D textureRaw;
	vks::Texture2D textureCalibrated;
public:
	struct {
		VkPipelineVertexInputStateCreateInfo inputState;
		std::vector<VkVertexInputBindingDescription> bindingDescriptions;
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
	} vertices;
};

int main() {
	return 0;
}