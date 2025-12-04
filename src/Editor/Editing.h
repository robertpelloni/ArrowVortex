#pragma once

#include <Core/Input.h>

#include <Managers/ChartMan.h>

namespace Vortex {

struct Editing : public InputHandler
{
	enum MirrorType
	{
		MIRROR_H,
		MIRROR_V,
		MIRROR_HV,
		MIRROR_LR, // For DDream "Mirror (L-R)" - likely horizontal (left/right) flip? No, standard Mirror H is L-R.
		// Wait, DDream has "Mirror (Full)", "Mirror (L-R)", "Mirror (U-D)".
		// StepMania usually has Mirror (reverses columns) and Flip (reverses Up/Down?).
		// MIRROR_H usually means reversing the column order 0..N -> N..0. This is "Mirror".
		// MIRROR_V usually means swapping Up with Down?
		// MIRROR_HV is both.
		// Let's assume MIRROR_H maps to "Mirror (L-R)" and MIRROR_V to "Mirror (U-D)".
		// And MIRROR_HV to "Mirror (Full)".
		// I'll check the implementation in Editing.cpp.
	};
	
	enum class VisualSyncAnchor
	{
		/// Visual sync shifts receptor's row
		RECEPTORS,
		/// Visual sync shifts closest snapping row at mouse cursor
		CURSOR,
	};

	static void create(XmrNode& settings);
	static void destroy();

	virtual void saveSettings(XmrNode& settings) = 0;

	/// Called by the editor when changes were made to the simfile.
	virtual void onChanges(int changes) = 0;

	virtual void drawGhostNotes() = 0;

	virtual void deleteSelection() = 0;
	
	virtual void changeNotesToType(NoteType type) = 0;
	virtual void changeMinesToType(NoteType type) = 0;
	virtual void changeFakesToType(NoteType type) = 0;
	virtual void changeLiftsToType(NoteType type) = 0;
	virtual void changeHoldsToType(NoteType type) = 0;
	virtual void changeHoldsToRolls() = 0;
	virtual void changePlayerNumber() = 0;

	virtual void mirrorNotes(MirrorType type) = 0;
	virtual void shuffleNotes(bool perRow) = 0;
	virtual void turnNotes(bool right) = 0;
	virtual void scaleNotes(int numerator, int denominator) = 0;

	virtual void insertRows(int row, int numRows, bool curChartOnly) = 0;

	virtual void convertCouplesToRoutine() = 0;
	virtual void convertRoutineToCouples() = 0;

	virtual void exportNotesAsLuaTable() = 0;

	virtual void toggleJumpToNextNote() = 0;
	virtual bool hasJumpToNextNote() = 0;

	virtual void toggleUndoRedoJump() = 0;
	virtual bool hasUndoRedoJump() = 0;

	virtual void toggleTimeBasedCopy() = 0;
	virtual bool hasTimeBasedCopy() = 0;

	virtual void toggleRecordMode() = 0;
	virtual bool isRecordMode() = 0;

	virtual void setVisualSyncAnchor(VisualSyncAnchor anchor) = 0;
	virtual VisualSyncAnchor getVisualSyncMode() = 0;
	virtual void injectBoundingBpmChange() = 0;
	virtual void shiftAnchorRowToMousePosition(bool is_destructive) = 0;
};

extern Editing* gEditing;

}; // namespace Vortex