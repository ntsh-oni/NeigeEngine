#pragma once
#include "../pipelines/DescriptorSet.h"
#include "../structs/UIStructs.h"
#include <queue>
#include <vector>

inline std::queue<Text> texts;
inline std::vector<Font> fonts;
inline std::vector<DescriptorSet> fontsDescriptorSets;
inline std::vector<bool> fontDescriptorSetUpToDate;