#pragma once

#include "../image/image.h"
#include "../core/assetLib.h"
#include <syscore/common.h>

template<typename T>
void ImageBuffer<T>::Serialize( Serializer* s )
{
	ImageBufferInterface::Serialize( s );
}
