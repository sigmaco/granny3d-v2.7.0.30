fn GTM_IsMaskableItem item =
(
    print item.name
	return (classof item == BoneGeometry or
	        classof item == Dummy or
	        classof item == Biped_Object)
)

-- read the current attribute definition into the gui
fn GrannyRebuildTrackList = 
(
	global GrannyTrackMaskAttribute
	global GTM_rollout
	trackMaskList = #()

	if GrannyTrackMaskAttribute == undefined do
	(
		-- try to find the attribute in an existing bone...
		allBones = for item in $objects where GTM_IsMaskableItem item == true collect item
		for bone in allBones do
		(
			myAtts = custAttributes.getDefs bone
			if myAtts != undefined do
			(
				for att in myAtts do
				(
					if (att.name == #GrannyTrackMasks) do
					(
						GrannyTrackMaskAttribute = att
						exit
					)
				)
			)

			if (GrannyTrackMaskAttribute != undefined) do
				exit
		)
	)

	if GrannyTrackMaskAttribute != undefined do
	(
		pblock = (custAttributes.getDefSource GrannyTrackMaskAttribute) as stringstream
	
		while (eof pblock == false) do
		(
			nextWord = readToken pblock
			if (nextWord == undefined) do
				exit

			if (findstring nextWord "TrackMask" == 1) then 
			(
				newName = ((substring nextWord 10 999)as name)
				if (newName != #_rollout) do
					append trackMaskList newName 
			)
		)	
	)
	return trackMaskList
)

-- rebuild the attribute definition
fn GrannyRebuildAttibute = 
(
global GTM_rollout
MaskList = GTM_rollout.GTM_Tracklist.items
	if (MaskList.count > 0) then
	(
	global GrannyTrackMaskAttribute
	command = "" as stringstream;
	format "global GrannyTrackMaskAttribute = attributes GrannyTrackMasks" to:command
	if GrannyTrackMaskAttribute != undefined do 
	(
	format " redefine:GrannyTrackMaskAttribute" to:command
	)
	format "\n(\n" to:command
-- the params section
	format "\nparameters main rollout:TrackMask_rollout\n" to:command
	format "\n(\n" to:command
	
	for mask in MaskList do
		(
		mask = mask as string
		if (mask.count > 1) do format "TrackMask% type:#float ui:spinner_%\n" mask mask to:command
		)
	format "\n)\n" to:command
-- the params section
	format "\nrollout TrackMask_rollout \"Granny Track Masks\"\n" to:command
	format "\n(\n" to:command
	for mask in MaskList do
		(
		mask = mask as string
		if (mask.count > 1) do  format "Spinner spinner_%\"%: \" type:#float\n" mask mask to:command
		)
	format "\n)\n)" to:command
	seek command 0;
	execute command

	)
)

-- delete the mask selected in the gui
fn GrannyDeleteTrackMask = 
(
global GTM_rollout
if (GTM_rollout.GTM_tracklist.Selection > 0) do
	(
	itemsArray = GTM_rollout.GTM_tracklist.items
	DeleteItem itemsArray GTM_rollout.GTM_tracklist.Selection
	GTM_rollout.GTM_tracklist.items = itemsArray
	GrannyRebuildAttibute ()
	)

)


-- rename selected mask to string "newItem"
fn GrannyRenameTrackMask newItem =
(
global GTM_rollout
if (GTM_rollout.GTM_tracklist.Selection > 0) do
	(
	allBones = for item in $objects where GTM_IsMaskableItem item == true collect item.name
	iterate = 1
	oldValue = #()
	-- cache old values
	oldAttrib = (".TrackMask" + GTM_rollout.GTM_tracklist.items[GTM_rollout.GTM_tracklist.Selection] )
	for bone in allBones do
	(
	try 
		(
		oldValue[iterate] = execute ("$" + bone + oldAttrib )
		)
	catch 
		(
		oldValue[iterate] = 0
		)
	iterate += 1
	)
	
	-- debug 
	print oldValue
	
	itemsArray = GTM_rollout.GTM_tracklist.items
	DeleteItem itemsArray GTM_rollout.GTM_tracklist.Selection
	GTM_rollout.GTM_tracklist.items = itemsArray
	GrannyAddTrackMask newItem;
	
	-- reapply  old values
	attrib = ".TrackMask" + (GrannyValidateName newItem)
	iterate = 1
	for bone in allBones do
	(
	try 
		(
		execute ("$" + bone + attrib + "= oldValue[iterate]")
		)
	catch 
		(
		)
	iterate +=1
	)

	)
)

-- for making sure name is OK
fn GrannyValidateName text = 
(
		local alphaNumeric = "qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM1234567890_"
		text = trimRight text
		text = trimLeft text
		for x = 1 to text.count do
		(
		if (findstring alphaNumeric text[x] == undefined) do text[x] = "_"
		)
		return text
)

fn GrannyAddTrackMask newName = 
(
global GTM_rollout
		if newName.count > 0 then	
		(
		oldList = GTM_rollout.GTM_tracklist.items
		newItem = GrannyValidateName (newName)
		newItem = newItem as name
		
		If (findItem oldList newItem == 0) then
			(
			oldList = append oldList newItem
			oldList = sort oldList
			GTM_rollout.GTM_tracklist.items = oldList
			GTM_rollout.GTM_newname.text = "";	
			GrannyRebuildAttibute ()	
			)
		)
)

fn GrannyApplyTrackMasks =
(
global GrannyTrackMaskAttribute
global GTM_rollout
allBones = for item in $objects where GTM_IsMaskableItem item == true collect item
for bone in allBones do
	(
		myAtts = custAttributes.getDefs bone
		if (myAtts == undefined or findItem myAtts GrannyTrackMaskAttribute == 0) do 
			(
			custAttributes.add bone GrannyTrackMaskAttribute
			)
	)

)

fn GrannyTrackMaskUI = 
(
global GTM_rollout
rollout GTM_rollout "Track masks" width:260 height:280
	(
		listBox GTM_TrackList "Track Masks" pos:[7,6] width:242 height:11 enabled:true
		editText GTM_newName "" text:"" pos:[71,180] width:180 height:26
		button GTM_add "Add" pos:[7,181] width:57 height:21
		button GTM_Delete "Delete" pos:[7,211] width:120 height:28
		button GTM_Rename "Rename" pos:[131,210] width:120 height:28
		button GTM_apply "Apply to all" pos:[8,244] width:120 height:28
		--button btn5 "Apply" pos:[131,244] width:120 height:28
	
		on GTM_newName entered text do
		(
		GrannyAddTrackMask( text )
		)
		
		on GTM_add pressed do
		(
		GrannyAddTrackMask(GTM_newName.text)
		)
		
		on GTM_Delete pressed do
		(
		GrannyDeleteTrackMask()
		)
	
		on GTM_Apply pressed do
		(
		GrannyApplyTrackMasks()
		destroyDialog GTM_rollout
		)
		-- same as double-clicking
	
		on GTM_Tracklist doubleClicked item do
		(
		-- this will eventually do the selection
		print GTM_tracklist.items[item]
		)		
		
		on GTM_rename pressed do
		(
			rollout GTM_renameWindow "Rename Track Mask" width:300 height:100
			(
				editText newNameField "" pos:[10,13] width:280 height:24
				button rename_OK "OK" pos:[170,60] width:120 height:24
				button rename_cancel "Cancel" pos:[20,60] width:120 height:24
				
				on rename_OK pressed do 
					(
					GrannyRenameTrackMask newNameField.text
					destroyDialog GTM_renameWindow
					
					)
				on rename_cancel pressed do
					(
					destroyDialog GTM_renameWindow
					)
			)
		createDialog GTM_renameWindow modal:true
		)

	)
createDialog GTM_rollout 268 288
GTM_rollout.GTM_TrackList.items = GrannyRebuildTrackList()
)
