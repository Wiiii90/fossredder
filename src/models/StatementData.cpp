#include "pch.h"
#include "models/StatementData.h"

StatementData::StatementData(const std::string& sourceFile,
	const std::vector<uint8_t>& sourceBytes,
	const RenderResult& renderArtifact,
	const std::vector<LayoutResult>& openCvArtifacts,
	const std::vector<std::vector<OcrTableDto>>& ocrArtifacts)
	: sourceFile(sourceFile), sourceBytes(sourceBytes), renderArtifact(renderArtifact), openCvArtifacts(openCvArtifacts), ocrArtifacts(ocrArtifacts)
{
}

const std::string& StatementData::getSourceFile() const { return sourceFile; }
const std::vector<uint8_t>& StatementData::getSourceBytes() const { return sourceBytes; }

const RenderResult& StatementData::getRenderArtifact() const { return renderArtifact; }
const std::vector<LayoutResult>& StatementData::getOpenCvArtifacts() const { return openCvArtifacts; }
const std::vector<std::vector<OcrTableDto>>& StatementData::getOcrArtifacts() const { return ocrArtifacts; }
