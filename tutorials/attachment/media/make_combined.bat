preprocessor CombineElements model.gr2 run.gr2 -output combined.gr2 -combine Animations -combine TrackGroups
preprocessor BonesPerMesh combined.gr2 -output combined.gr2 -bonelimit 64
preprocessor Compress combined.gr2 -output combined.gr2
