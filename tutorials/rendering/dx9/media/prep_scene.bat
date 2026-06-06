preprocessor CleanMaterials ../../../media/pixo_dance_source.gr2 -output pixo_dance.gr2
preprocessor KillUncommonData pixo_dance.gr2 -output pixo_dance.gr2
preprocessor MeshIndices16Bit pixo_dance.gr2 -output pixo_dance.gr2
preprocessor VertexCacheOptimize pixo_dance.gr2 -output pixo_dance.gr2
preprocessor BonesPerMesh pixo_dance.gr2 -output pixo_dance.gr2 -bonelimit 64
preprocessor RemoveVectorTracks pixo_dance.gr2 -output pixo_dance.gr2

REM Note that KillUncommonData strips the ExtendedData, which contains
REM our camera information, so we store it in a seperate file, and
REM combine it back in after we're done processing the main scene
preprocessor CombineElements pixo_dance.gr2 ../../../media/pixo_dance_camera.gr2 -output pixo_dance.gr2 -combine Skeletons -combine Models

preprocessor ResectionFile pixo_dance.gr2 -output pixo_dance.gr2
preprocessor Compress pixo_dance.gr2 -output pixo_dance.gr2
