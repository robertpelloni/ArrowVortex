#pragma once

<<<<<<< HEAD
#include <Core/Core.h>
=======
#include <Precomp.h>
>>>>>>> origin/fietsemaker-beta

namespace AV {
namespace SimfileConstants {

<<<<<<< HEAD
/// Determines how the selection is modified.
enum SelectModifier {
    SELECT_SET,  ///< Replace the current selection.
    SELECT_ADD,  ///< Add to the current selection.
    SELECT_SUB,  ///< Subtract from the current selection.
};

/// Simfile limitations/constants.
enum SimConstants {
    SIM_MAX_COLUMNS = 32,
    SIM_MAX_PLAYERS = 16,
    SIM_DEFAULT_BPM = 120,
};

/// Supported difficulties.
enum Difficulty {
    DIFF_BEGINNER,
    DIFF_EASY,
    DIFF_MEDIUM,
    DIFF_HARD,
    DIFF_CHALLENGE,
    DIFF_EDIT,

    NUM_DIFFICULTIES
};

/// Represents a row/column index.
struct RowCol {
    int row, col;
};

// Represents a generic tag/value property.
struct Property {
    std::string tag, val;
};

/// Represents a single note.
struct Note {
    int row, endrow;
    uint32_t col : 8;
    uint32_t player : 4;
    uint32_t type : 4;
    uint32_t quant : 8;
};

};  // namespace Vortex
=======
constexpr int MaxColumns = 32;
constexpr int MaxPlayers = 16;
constexpr int MaxKeysounds = 4095;

} // Simfile constants

// Supported note types.
enum class NoteType
{
	None,

	Step,
	Mine,
	Hold,
	Roll,
	Lift,
	Fake,
	AutomaticKeysound,

	Count
};

// Represents a row/column index.
struct RowCol
{
	Row row;
	int col;
};

// Represents a single note.
struct Note
{
	Note()
		: row(0)
		, endRow(0)
		, type(0)
		, player(0)
		, keysoundId(0)
		, isWarped(0)
		, unused(0)
	{
	}

	Note(Row row, Row endRow, NoteType type, uint player, uint keysoundId)
		: row(row)
		, endRow(endRow)
		, type((uint)type)
		, player(player)
		, keysoundId(keysoundId)
		, isWarped(0)
		, unused(0)
	{
	}

	Row row;
	Row endRow;

	uint type : 4;
	uint player : 4;
	uint keysoundId : 12;
	uint isWarped : 1;
	uint unused : 3;
};

} // namespace AV
>>>>>>> origin/fietsemaker-beta
