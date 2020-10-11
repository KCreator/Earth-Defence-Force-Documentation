std::vector<uint8_t> out;
std::vector<uint8_t> temp;
int16_t mainbuf[4096];
uint16_t bufPos = 4078;
size_t streamPos = 0;
uint8_t bits = 0;

for (size_t i = 0; i < _countof(mainbuf); i++) {
	if (i < bufPos) {
		mainbuf[i] = 0;
	} else {
		// Mark end of buffer as uninitialized
		mainbuf[i] = -1;
	}
}

while (streamPos < dataSize) {
	bits = 0;
	for (size_t i = 0; i < 8; i++) {
		if (streamPos >= dataSize) {
			break;
		}
		size_t bestPos = 0;
		size_t bestLen = 0;
		// Try to find match in buffer
		// TODO: Properly support repeating data
		for (size_t jo = 0; jo < _countof(mainbuf); jo++) {
			uint16_t j = (bufPos - jo) & 0xFFF;
			if (mainbuf[j] == (int16_t)(uint16_t)data[streamPos]) {
				size_t matchLen = 0;
				for (size_t k = 0; k < 18; k++) {
					if ((streamPos + k) < dataSize && ((j + k) & 0xFFF) != bufPos && mainbuf[(j + k) & 0xFFF] == (int16_t)(uint16_t)data[streamPos + k]) {
						matchLen = k + 1;
					} else {
						break;
					}
				}
				if (matchLen > bestLen) {
					bestLen = matchLen;
					bestPos = j;
				}
			}
		}
		// Repeating byte check
		if (mainbuf[(bufPos - 1) & 0xFFF] == (int16_t)(uint16_t)data[streamPos]) {
			size_t matchLen = 0;
			for (size_t k = 0; k < 18; k++) {
				if ((streamPos + k) < dataSize && mainbuf[(bufPos - 1) & 0xFFF] == (int16_t)(uint16_t)data[streamPos + k]) {
					matchLen = k + 1;
				} else {
					break;
				}
			}
			if (matchLen > bestLen) {
				bestLen = matchLen;
				bestPos = (bufPos - 1) & 0xFFF;
			}
		}
		// Is copy viable?
		if (bestLen >= 3) {
			// Write copy data
			uint16_t copyVal = bestLen - 3;
			copyVal |= bestPos << 4;
			temp.push_back(copyVal >> 8);
			temp.push_back(copyVal & 0xFF);
			for (size_t j = 0; j < bestLen; j++) {
				mainbuf[bufPos] = data[streamPos];
				bufPos = (bufPos + 1) & 0xFFF;
				streamPos++;
			}
		} else {
			// Copy from input
			temp.push_back(data[streamPos]);
			mainbuf[bufPos] = data[streamPos];
			bufPos = (bufPos + 1) & 0xFFF;
			streamPos++;
			bits |= 1 << i;
		}
	}
	out.push_back(bits);
	out.insert(out.end(), temp.begin(), temp.end());
	temp.clear();
}