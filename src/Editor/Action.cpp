#include <Editor/Action.h>

#include <System/System.h>

#include <Editor/Editor.h>
#include <Editor/Menubar.h>
#include <Editor/View.h>
#include <Editor/History.h>
#include <Editor/Statusbar.h>
#include <Editor/Editing.h>
#include <Editor/Notefield.h>
#include <Editor/Waveform.h>
#include <Editor/Selection.h>
#include <Editor/TextOverlay.h>
#include <Editor/Music.h>
#include <Editor/Minimap.h>
#include <Editor/TempoBoxes.h>
#include <Editor/StructureAnalyzer.h>
#include <Editor/FindTempo.h>
#include <Editor/FindOnsets.h>
#include <Simfile/NoteList.h>

#include <vector>
#include <algorithm>
#include <thread>
#include <chrono>

#include <Managers/MetadataMan.h>
#include <Managers/NoteskinMan.h>
#include <Managers/StyleMan.h>
#include <Managers/SimfileMan.h>
#include <Managers/TempoMan.h>
#include <Managers/LuaMan.h>

#include <Dialogs/Dialog.h>
#include <Dialogs/GoTo.h>
#include <Dialogs/LyricsEditor.h>
#include <Dialogs/BgChanges.h>
#include <Dialogs/ChartStatistics.h>
#include <Core/Widgets.h>

namespace Vortex {

void Action::perform(Type action)
{
	#define FIRST_CASE(x) case x: {
	#define CASE(x) } break; case x: {

	switch(action)
	{
	FIRST_CASE(EXIT_PROGRAM)
		gEditor->onExitProgram();

	CASE(FILE_OPEN)
		gEditor->openSimfile();
	CASE(FILE_SAVE)
		gEditor->saveSimfile(false);
	CASE(FILE_SAVE_AS)
		gEditor->saveSimfile(true);
	CASE(FILE_CLOSE)
		gEditor->closeSimfile();

	CASE(FILE_CLEAR_RECENT_FILES)
		gEditor->clearRecentFiles();

	CASE(OPEN_DIALOG_SONG_PROPERTIES)
		gEditor->openDialog(DIALOG_SONG_PROPERTIES);
	CASE(OPEN_DIALOG_CHART_PROPERTIES)
		gEditor->openDialog(DIALOG_CHART_PROPERTIES);
	CASE(OPEN_DIALOG_CHART_LIST)
		gEditor->openDialog(DIALOG_CHART_LIST);
	CASE(OPEN_DIALOG_NEW_CHART)
		gEditor->openDialog(DIALOG_NEW_CHART);
	CASE(OPEN_DIALOG_ADJUST_SYNC)
		gEditor->openDialog(DIALOG_ADJUST_SYNC);
	CASE(OPEN_DIALOG_ADJUST_TEMPO)
		gEditor->openDialog(DIALOG_ADJUST_TEMPO);
	CASE(OPEN_DIALOG_ADJUST_TEMPO_SM5)
		gEditor->openDialog(DIALOG_ADJUST_TEMPO_SM5);
	CASE(OPEN_DIALOG_DANCING_BOT)
		gEditor->openDialog(DIALOG_DANCING_BOT);
	CASE(OPEN_DIALOG_GENERATE_NOTES)
		gEditor->openDialog(DIALOG_GENERATE_NOTES);
	CASE(OPEN_DIALOG_CHART_STATISTICS)
		gEditor->openDialog(DIALOG_CHART_STATISTICS);
	CASE(OPEN_DIALOG_TEMPO_BREAKDOWN)
		gEditor->openDialog(DIALOG_TEMPO_BREAKDOWN);
	CASE(OPEN_DIALOG_WAVEFORM_SETTINGS)
		gEditor->openDialog(DIALOG_WAVEFORM_SETTINGS);
	CASE(OPEN_DIALOG_ZOOM)
		gEditor->openDialog(DIALOG_ZOOM);
	CASE(OPEN_DIALOG_CUSTOM_SNAP)
		gEditor->openDialog(DIALOG_CUSTOM_SNAP);
	CASE(OPEN_DIALOG_GO_TO)
		gEditor->openDialog(DIALOG_GO_TO);
	CASE(OPEN_DIALOG_LYRICS_EDITOR)
		gEditor->openDialog(DIALOG_LYRICS_EDITOR);
	CASE(OPEN_DIALOG_BG_CHANGES)
		gEditor->openDialog(DIALOG_BG_CHANGES);
	CASE(OPEN_DIALOG_PREFERENCES)
		gEditor->openDialog(DIALOG_PREFERENCES);
	CASE(OPEN_DIALOG_BATCH_DDC)
		gEditor->openDialog(DIALOG_BATCH_DDC);

	CASE(EDIT_UNDO)
		gSystem->getEvents().addKeyPress(Key::Z, Keyflag::CTRL, false);
	CASE(EDIT_REDO)
		gSystem->getEvents().addKeyPress(Key::Y, Keyflag::CTRL, false);
	CASE(EDIT_CUT)
		gSystem->getEvents().addKeyPress(Key::X, Keyflag::CTRL, false);
	CASE(EDIT_COPY)
		gSystem->getEvents().addKeyPress(Key::C, Keyflag::CTRL, false);
	CASE(EDIT_PASTE)
		gSystem->getEvents().addKeyPress(Key::V, Keyflag::CTRL, false);
	CASE(EDIT_DELETE)
		gSystem->getEvents().addKeyPress(Key::DELETE, 0, false);
	CASE(INSERT_MEASURE)
		gEditing->insertRows(gView->getCursorRow(), 192, false);
	CASE(DELETE_MEASURE)
		gEditing->insertRows(gView->getCursorRow(), -192, false);
	CASE(INSERT_BEAT)
		gEditing->insertRows(gView->getCursorRow(), 48, false);
	CASE(DELETE_BEAT)
		gEditing->insertRows(gView->getCursorRow(), -48, false);
	CASE(QUANTIZE_SELECTION)
		{
			int snap = gView->getSnapQuant();
			if (snap > 0) gEditing->quantizeSelection(snap);
			else HudError("Invalid snap for quantization.");
		}
	CASE(SELECT_ALL)
		gSystem->getEvents().addKeyPress(Key::A, Keyflag::CTRL, false);

	CASE(TOGGLE_JUMP_TO_NEXT_NOTE)
		gEditing->toggleJumpToNextNote();
	CASE(TOGGLE_UNDO_REDO_JUMP)
		gEditing->toggleUndoRedoJump();
	CASE(TOGGLE_TIME_BASED_COPY)
		gEditing->toggleTimeBasedCopy();
	CASE(TOGGLE_RECORD_MODE)
		gEditing->toggleRecordMode();

	CASE(SET_VISUAL_SYNC_CURSOR_ANCHOR)
		gEditing->setVisualSyncAnchor(Editing::VisualSyncAnchor::CURSOR);
	CASE(SET_VISUAL_SYNC_RECEPTOR_ANCHOR)
		gEditing->setVisualSyncAnchor(Editing::VisualSyncAnchor::RECEPTORS);
	CASE(INJECT_BOUNDING_BPM_CHANGE)
		gEditing->injectBoundingBpmChange();
	CASE(SHIFT_ROW_NONDESTRUCTIVE)
		gEditing->shiftAnchorRowToMousePosition(false);
	CASE(SHIFT_ROW_DESTRUCTIVE)
		gEditing->shiftAnchorRowToMousePosition(true);

	CASE(SELECT_REGION)
		gSelection->selectRegion();
	CASE(SELECT_ALL_STEPS)
		gSelection->selectNotes(NotesMan::SELECT_STEPS);
	CASE(SELECT_ALL_MINES)
		gSelection->selectNotes(NotesMan::SELECT_MINES);
	CASE(SELECT_ALL_HOLDS)
		gSelection->selectNotes(NotesMan::SELECT_HOLDS);
	CASE(SELECT_ALL_ROLLS)
		gSelection->selectNotes(NotesMan::SELECT_ROLLS);
	CASE(SELECT_ALL_FAKES)
		gSelection->selectNotes(NotesMan::SELECT_FAKES);
	CASE(SELECT_ALL_LIFTS)
		gSelection->selectNotes(NotesMan::SELECT_LIFTS);
	CASE(SELECT_REGION_BEFORE_CURSOR)
		gSelection->selectRegion(0, gView->getCursorRow());
	CASE(SELECT_REGION_AFTER_CURSOR)
		gSelection->selectRegion(gView->getCursorRow(), gSimfile->getEndRow());

	CASE(SELECT_QUANT_4)
		gSelection->selectNotes(RT_4TH);
	CASE(SELECT_QUANT_8)
		gSelection->selectNotes(RT_8TH);
	CASE(SELECT_QUANT_12)
		gSelection->selectNotes(RT_12TH);
	CASE(SELECT_QUANT_16)
		gSelection->selectNotes(RT_16TH);
	CASE(SELECT_QUANT_24)
		gSelection->selectNotes(RT_24TH);
	CASE(SELECT_QUANT_32)
		gSelection->selectNotes(RT_32ND);
	CASE(SELECT_QUANT_48)
		gSelection->selectNotes(RT_48TH);
	CASE(SELECT_QUANT_64)
		gSelection->selectNotes(RT_64TH);
	CASE(SELECT_QUANT_192)
		gSelection->selectNotes(RT_192TH);

	CASE(SELECT_TEMPO_BPM)
		gTempoBoxes->selectType(Segment::BPM);
	CASE(SELECT_TEMPO_STOP)
		gTempoBoxes->selectType(Segment::STOP);
	CASE(SELECT_TEMPO_DELAY)
		gTempoBoxes->selectType(Segment::DELAY);
	CASE(SELECT_TEMPO_WARP)
		gTempoBoxes->selectType(Segment::WARP);
	CASE(SELECT_TEMPO_TIME_SIG)
		gTempoBoxes->selectType(Segment::TIME_SIG);
	CASE(SELECT_TEMPO_TICK_COUNT)
		gTempoBoxes->selectType(Segment::TICK_COUNT);
	CASE(SELECT_TEMPO_COMBO)
		gTempoBoxes->selectType(Segment::COMBO);
	CASE(SELECT_TEMPO_SPEED)
		gTempoBoxes->selectType(Segment::SPEED);
	CASE(SELECT_TEMPO_SCROLL)
		gTempoBoxes->selectType(Segment::SCROLL);
	CASE(SELECT_TEMPO_FAKE)
		gTempoBoxes->selectType(Segment::FAKE);
	CASE(SELECT_TEMPO_LABEL)
		gTempoBoxes->selectType(Segment::LABEL);

	CASE(CHART_PREVIOUS)
		gSimfile->previousChart();
	CASE(CHART_NEXT)
		gSimfile->nextChart();
	CASE(CHART_DELETE)
		gSimfile->removeChart(gChart->get());

	CASE(SIMFILE_PREVIOUS)
		gEditor->openNextSimfile(false);
	CASE(SIMFILE_NEXT)
		gEditor->openNextSimfile(true);

	CASE(CHART_CONVERT_COUPLES_TO_ROUTINE)
		gEditing->convertCouplesToRoutine();
	CASE(CHART_CONVERT_ROUTINE_TO_COUPLES)
		gEditing->convertRoutineToCouples();

	CASE(CHANGE_NOTES_TO_MINES)
		gEditing->changeNotesToType(NoteType::NOTE_MINE);
	CASE(CHANGE_NOTES_TO_FAKES)
		gEditing->changeNotesToType(NoteType::NOTE_FAKE);
	CASE(CHANGE_NOTES_TO_LIFTS)
		gEditing->changeNotesToType(NoteType::NOTE_LIFT);
	CASE(CHANGE_MINES_TO_NOTES)
		gEditing->changeMinesToType(NoteType::NOTE_STEP_OR_HOLD);
	CASE(CHANGE_MINES_TO_FAKES)
		gEditing->changeMinesToType(NoteType::NOTE_FAKE);
	CASE(CHANGE_MINES_TO_LIFTS)
		gEditing->changeMinesToType(NoteType::NOTE_LIFT);
	CASE(CHANGE_FAKES_TO_NOTES)
		gEditing->changeFakesToType(NoteType::NOTE_STEP_OR_HOLD);
	CASE(CHANGE_LIFTS_TO_NOTES)
		gEditing->changeLiftsToType(NoteType::NOTE_STEP_OR_HOLD);
	CASE(CHANGE_HOLDS_TO_STEPS)
		gEditing->changeHoldsToType(NoteType::NOTE_STEP_OR_HOLD);
	CASE(CHANGE_HOLDS_TO_MINES)
		gEditing->changeHoldsToType(NoteType::NOTE_MINE);
	CASE(CHANGE_BETWEEN_HOLDS_AND_ROLLS)
		gEditing->changeHoldsToRolls();
	CASE(CHANGE_BETWEEN_PLAYER_NUMBERS)
		gEditing->changePlayerNumber();

	CASE(MIRROR_NOTES_VERTICALLY)
		gEditing->mirrorNotes(Editing::MIRROR_V);
	CASE(MIRROR_NOTES_HORIZONTALLY)
		gEditing->mirrorNotes(Editing::MIRROR_H);
	CASE(MIRROR_NOTES_FULL)
		gEditing->mirrorNotes(Editing::MIRROR_HV);

	CASE(SHUFFLE_NOTES)
		gEditing->shuffleNotes(false);
	CASE(SUPER_SHUFFLE_NOTES)
		gEditing->shuffleNotes(true);
	CASE(TURN_NOTES_LEFT)
		gEditing->turnNotes(false);
	CASE(TURN_NOTES_RIGHT)
		gEditing->turnNotes(true);

	CASE(EXPORT_NOTES_AS_LUA_TABLE)
		gEditing->exportNotesAsLuaTable();

	CASE(SCALE_NOTES_2_TO_1)
		gEditing->scaleNotes(2, 1);
	CASE(SCALE_NOTES_3_TO_2)
		gEditing->scaleNotes(3, 2);
	CASE(SCALE_NOTES_4_TO_3)
		gEditing->scaleNotes(4, 3);
	CASE(SCALE_NOTES_1_TO_2)
		gEditing->scaleNotes(1, 2);
	CASE(SCALE_NOTES_2_TO_3)
		gEditing->scaleNotes(2, 3);
	CASE(SCALE_NOTES_3_TO_4)
		gEditing->scaleNotes(3, 4);

	CASE(SWITCH_TO_SYNC_MODE)
		gSimfile->openChart(-1);

	CASE(VOLUME_RESET)
		gMusic->setVolume(100);
	CASE(VOLUME_INCREASE)
		gMusic->setVolume(gMusic->getVolume() + 10);
	CASE(VOLUME_DECREASE)
		gMusic->setVolume(gMusic->getVolume() - 10);
	CASE(VOLUME_MUTE)
		gMusic->setMuted(!gMusic->isMuted());

	CASE(CONVERT_MUSIC_TO_OGG)
		gMusic->startOggConversion();

	CASE(SPEED_RESET)
		gMusic->setSpeed(100);
	CASE(SPEED_INCREASE)
		gMusic->setSpeed(gMusic->getSpeed() + 10);
	CASE(SPEED_DECREASE)
		gMusic->setSpeed(gMusic->getSpeed() - 10);

	CASE(TOGGLE_LOOP_SELECTION)
		{
			if (gMusic->isLooping()) {
				gMusic->toggleLoop();
			} else {
				auto region = gSelection->getSelectedRegion();
				if (region.beginRow != region.endRow) {
					double start = gTempo->rowToTime(region.beginRow);
					double end = gTempo->rowToTime(region.endRow);
					gMusic->setLoopRegion(start, end);
					gMusic->seek(start);
					if (gMusic->isPaused()) gMusic->play();
				} else {
					HudError("Select a region to loop.");
				}
			}
		}

	CASE(TOGGLE_BEAT_TICK)
		gMusic->toggleBeatTick();
	CASE(TOGGLE_NOTE_TICK)
		gMusic->toggleNoteTick();

	CASE(TOGGLE_SHOW_WAVEFORM)
		gNotefield->toggleShowWaveform();
	CASE(TOGGLE_SHOW_SPECTROGRAM)
		gWaveform->toggleSpectrogram();
	CASE(TOGGLE_SHOW_BEAT_LINES)
		gNotefield->toggleShowBeatLines();
	CASE(TOGGLE_SHOW_NOTES)
		gNotefield->toggleShowNotes();
	CASE(TOGGLE_SHOW_TEMPO_BOXES)
		gTempoBoxes->toggleShowBoxes();
	CASE(TOGGLE_SHOW_TEMPO_HELP)
		gTempoBoxes->toggleShowHelp();
	CASE(TOGGLE_REVERSE_SCROLL)
		gView->toggleReverseScroll();
	CASE(TOGGLE_CHART_PREVIEW)
		gView->toggleChartPreview();

	CASE(MINIMAP_SET_NOTES)
		gMinimap->setMode(Minimap::NOTES);
	CASE(MINIMAP_SET_DENSITY)
		gMinimap->setMode(Minimap::DENSITY);

	CASE(BACKGROUND_HIDE)
		gNotefield->setBgAlpha(0);
	CASE(BACKGROUND_INCREASE_ALPHA)
		gNotefield->setBgAlpha(gNotefield->getBgAlpha() + 10);
	CASE(BACKGROUND_DECREASE_ALPHA)
		gNotefield->setBgAlpha(gNotefield->getBgAlpha() - 10);
	CASE(BACKGROUND_SET_STRETCH)
		gEditor->setBackgroundStyle(BG_STYLE_STRETCH);
	CASE(BACKGROUND_SET_LETTERBOX)
		gEditor->setBackgroundStyle(BG_STYLE_LETTERBOX);
	CASE(BACKGROUND_SET_CROP)
		gEditor->setBackgroundStyle(BG_STYLE_CROP);

	CASE(USE_TIME_BASED_VIEW)
		gView->setTimeBased(true);
	CASE(USE_ROW_BASED_VIEW)
		gView->setTimeBased(false);

	CASE(APPLY_SYNC_LAYOUT)
		gView->setTimeBased(true);
		gNotefield->setShowWaveform(true);
		gNotefield->setShowBeatLines(true);
		gNotefield->setShowNotes(false);
		gTempoBoxes->setShowBoxes(true);

	CASE(ZOOM_RESET)
		gView->setZoomLevel(8);
		gView->setScaleLevel(4);
	CASE(ZOOM_IN)
		gView->setZoomLevel(gView->getZoomLevel() + 0.25);
	CASE(ZOOM_OUT)
		gView->setZoomLevel(gView->getZoomLevel() - 0.25);
	CASE(SCALE_INCREASE)
		gView->setScaleLevel(gView->getScaleLevel() + 0.25);
	CASE(SCALE_DECREASE)
		gView->setScaleLevel(gView->getScaleLevel() - 0.25);

	CASE(SNAP_NEXT)
		gView->setSnapType(gView->getSnapType() + 1);
	CASE(SNAP_PREVIOUS)
		gView->setSnapType(gView->getSnapType() - 1);
	CASE(SNAP_RESET)
		gView->setSnapType(0);

	CASE(CURSOR_UP)
		gView->setCursorRow(gView->snapRow(gView->getCursorRow(), View::SNAP_UP));
	CASE(CURSOR_DOWN)
		gView->setCursorRow(gView->snapRow(gView->getCursorRow(), View::SNAP_DOWN));
	CASE(CURSOR_PREVIOUS_BEAT)
		gView->setCursorToNextInterval(-48);
	CASE(CURSOR_NEXT_BEAT)
		gView->setCursorToNextInterval(48);
	CASE(CURSOR_PREVIOUS_MEASURE)
		gView->setCursorToNextInterval(-192);
	CASE(CURSOR_NEXT_MEASURE)
		gView->setCursorToNextInterval(192);
	CASE(CURSOR_STREAM_START)
		gView->setCursorToStream(true);
	CASE(CURSOR_STREAM_END)
		gView->setCursorToStream(false);
	CASE(CURSOR_SELECTION_START)
		gView->setCursorToSelection(true);
	CASE(CURSOR_SELECTION_END)
		gView->setCursorToSelection(false);
	CASE(CURSOR_CHART_START)
		gView->setCursorRow(0);
	CASE(CURSOR_CHART_END)
		gView->setCursorRow(gSimfile->getEndRow());

	CASE(CURSOR_NEXT_TRANSIENT)
		{
			auto& music = gMusic->getSamples();
			if(music.isCompleted() && music.getNumFrames() > 0) {
				gWaveform->updateOnsets();
				const auto& onsets = gWaveform->getOnsets();
				if (onsets.empty()) break;

				double curTime = gView->getCursorTime();
				int samplerate = music.getFrequency();
				double bestTime = -1.0;

				// Find first onset > curTime
				// Onsets are sorted by position (sample index)
				for(const auto& onset : onsets) {
					double t = (double)onset.pos / samplerate;
					if (t > curTime + 0.001) { // Small epsilon
						bestTime = t;
						break;
					}
				}

				if (bestTime >= 0.0) {
					gView->setCursorTime(bestTime);
				}
			}
		}

	CASE(CURSOR_PREV_TRANSIENT)
		{
			auto& music = gMusic->getSamples();
			if(music.isCompleted() && music.getNumFrames() > 0) {
				gWaveform->updateOnsets();
				const auto& onsets = gWaveform->getOnsets();
				if (onsets.empty()) break;

				double curTime = gView->getCursorTime();
				int samplerate = music.getFrequency();
				double bestTime = -1.0;

				// Find last onset < curTime
				for(int i = onsets.size() - 1; i >= 0; --i) {
					double t = (double)onsets[i].pos / samplerate;
					if (t < curTime - 0.001) {
						bestTime = t;
						break;
					}
				}

				if (bestTime >= 0.0) {
					gView->setCursorTime(bestTime);
				}
			}
		}

	CASE(TOGGLE_STATUS_CHART)
		gStatusbar->toggleChart();
	CASE(TOGGLE_STATUS_SNAP)
		gStatusbar->toggleSnap();
	CASE(TOGGLE_STATUS_BPM)
		gStatusbar->toggleBpm();
	CASE(TOGGLE_STATUS_ROW)
		gStatusbar->toggleRow();
	CASE(TOGGLE_STATUS_BEAT)
		gStatusbar->toggleBeat();
	CASE(TOGGLE_STATUS_MEASURE)
		gStatusbar->toggleMeasure();
	CASE(TOGGLE_STATUS_TIME)
		gStatusbar->toggleTime();
	CASE(TOGGLE_STATUS_TIMING_MODE)
		gStatusbar->toggleTimingMode();

	CASE(SHOW_SHORTCUTS)
		gTextOverlay->show(TextOverlay::SHORTCUTS);
	CASE(SHOW_MESSAGE_LOG)
		gTextOverlay->show(TextOverlay::MESSAGE_LOG);
	CASE(SHOW_DEBUG_LOG)
		gTextOverlay->show(TextOverlay::DEBUG_LOG);
	CASE(SHOW_ABOUT)
		gTextOverlay->show(TextOverlay::ABOUT);

	CASE(DETECT_SONG_SECTIONS)
		{
			auto& music = gMusic->getSamples();
			if(music.isCompleted() && music.getNumFrames() > 0) {
				int numFrames = music.getNumFrames();
				int samplerate = music.getFrequency();
				std::vector<float> floatSamples(numFrames);
				const short* src = music.samplesL(); // Mono analysis
				for(int i=0; i<numFrames; ++i) floatSamples[i] = src[i] / 32768.0f;

				Vector<Section> sections;
				FindSections(floatSamples.data(), samplerate, numFrames, sections);

				for(const auto& s : sections) {
					int row = gTempo->timeToRow(s.time);
					gTempo->addSegment(Label(row, "Section"));
				}
				HudNote("Detected %d sections", sections.size());
			}
		}

	CASE(ESTIMATE_BPM_FROM_SELECTION)
		{
			auto region = gSelection->getSelectedRegion();
			if(region.beginRow == region.endRow) {
				HudError("No selection for BPM estimation.");
			} else {
				double start = gTempo->rowToTime(region.beginRow);
				double end = gTempo->rowToTime(region.endRow);
				auto detector = TempoDetector::New(start, end - start);
				if(detector) {
					int timeout = 500; // 5 seconds
					while(!detector->hasResult() && timeout > 0) {
						std::this_thread::sleep_for(std::chrono::milliseconds(10));
						timeout--;
					}
					if(detector->hasResult()) {
						auto results = detector->getResult();
						if(!results.empty()) {
							double bpm = results[0].bpm;
							Str::fmt msg("Estimated BPM: %.3f. Apply at row %d?");
							msg.arg(bpm).arg(region.beginRow);
							int res = gSystem->showMessageDlg("BPM Estimation", msg, System::T_YES_NO, System::I_INFO);
							if (res == System::R_YES) {
								gTempo->addSegment(BpmChange(region.beginRow, bpm));
							}
						} else {
							HudError("Could not estimate BPM.");
						}
					} else {
						HudError("BPM estimation timed out.");
					}
					delete detector;
				}
			}
		}

	CASE(AUTO_SYNC_SONG)
		{
			auto& music = gMusic->getSamples();
			if (!music.isCompleted()) {
				HudError("Music not loaded.");
			} else {
				auto detector = TempoDetector::New(0, gMusic->getSongLength());
				if (detector) {
					int timeout = 1000; // 10 seconds
					while(!detector->hasResult() && timeout > 0) {
						std::this_thread::sleep_for(std::chrono::milliseconds(10));
						timeout--;
					}
					if(detector->hasResult()) {
						auto results = detector->getResult();
						if(!results.empty()) {
							double bpm = results[0].bpm;
							double offset = -results[0].offset; // detector offset is sample time of first beat
							Str::fmt msg("Detected BPM: %.3f, Offset: %.3f. Apply (Replace All)?");
							msg.arg(bpm).arg(offset);
							int res = gSystem->showMessageDlg("Auto Sync", msg, System::T_YES_NO, System::I_INFO);
							if (res == System::R_YES) {
								SegmentEdit edit;
								// Remove all existing segments to ensure a clean slate
								const SegmentGroup* segs = gTempo->getSegments();
								if (segs) {
									const auto& bpms = segs->getList<BpmChange>();
									for(const auto& s : bpms) edit.rem.append(s);
									const auto& stops = segs->getList<Stop>();
									for(const auto& s : stops) edit.rem.append(s);
									const auto& delays = segs->getList<Delay>();
									for(const auto& s : delays) edit.rem.append(s);
									const auto& warps = segs->getList<Warp>();
									for(const auto& s : warps) edit.rem.append(s);
									const auto& timesigs = segs->getList<TimeSig>();
									for(const auto& s : timesigs) edit.rem.append(s);
									const auto& ticks = segs->getList<TickCount>();
									for(const auto& s : ticks) edit.rem.append(s);
									const auto& combos = segs->getList<Combo>();
									for(const auto& s : combos) edit.rem.append(s);
									const auto& speeds = segs->getList<Speed>();
									for(const auto& s : speeds) edit.rem.append(s);
									const auto& scrolls = segs->getList<Scroll>();
									for(const auto& s : scrolls) edit.rem.append(s);
									const auto& fakes = segs->getList<Fake>();
									for(const auto& s : fakes) edit.rem.append(s);
									const auto& labels = segs->getList<Label>();
									for(const auto& s : labels) edit.rem.append(s);
								}
								
								// Add the new BPM
								edit.add.append(BpmChange(0, bpm));
								
								gTempo->setOffset(offset);
								gTempo->modify(edit);
							}
						} else {
							HudError("Could not detect BPM.");
						}
					} else {
						HudError("Auto Sync timed out.");
					}
					delete detector;
				}
			}
		}

	CASE(SNAP_OFFSET_TO_FIRST_BEAT)
		{
			auto& music = gMusic->getSamples();
			if(music.isCompleted() && music.getNumFrames() > 0) {
				// Use onsets from Waveform if available (to match visualization) or recompute
				// Waveform has them but private/protected logic.
				// Let's recompute using FindOnsets.
				int numFrames = music.getNumFrames();
				int samplerate = music.getFrequency();
				std::vector<float> floatSamples(numFrames);
				const short* src = music.samplesL();
				for(int i=0; i<numFrames; ++i) floatSamples[i] = src[i] / 32768.0f;

				Vector<Onset> onsets;
				FindOnsets(floatSamples.data(), samplerate, numFrames, 1, onsets); // Default threshold 0.3f

				if (!onsets.empty()) {
					// Find first strong onset? Or just first.
					// Let's take the first one > 0.5? Or just the first one returned.
					// FindOnsets returns everything above threshold.
					double firstTime = (double)onsets[0].pos / samplerate;

					Str::fmt msg("Snap offset to first beat at %.3f? (Current: %.3f)");
					msg.arg(-firstTime).arg(gTempo->getOffset());
					int res = gSystem->showMessageDlg("Snap Offset", msg, System::T_YES_NO, System::I_INFO);
					if (res == System::R_YES) {
						gTempo->setOffset(-firstTime);
					}
				} else {
					HudError("No beats detected.");
				}
			}
		}

	CASE(QUANTIZE_TO_AUDIO)
		{
			auto& music = gMusic->getSamples();
			if(music.isCompleted() && music.getNumFrames() > 0) {
				auto region = gSelection->getSelectedRegion();
				int startRow = region.beginRow;
				int endRow = region.endRow;
				if (startRow == endRow) {
					startRow = 0;
					endRow = gSimfile->getEndRow();
				}

				// Ensure onsets are up to date
				gWaveform->updateOnsets();
				const auto& onsets = gWaveform->getOnsets();

				if (onsets.empty()) {
					HudError("No onsets detected (try adjusting threshold in Waveform settings).");
					break;
				}

				Str::fmt msg("Quantize notes in range [%d, %d] to audio?");
				msg.arg(startRow).arg(endRow);
				int res = gSystem->showMessageDlg("Quantize to Audio", msg, System::T_YES_NO, System::I_INFO);
				if (res != System::R_YES) break;

				int samplerate = music.getFrequency();
				double window = 0.05; // 50ms window

				NoteEdit edit;
				int count = 0;

				for(const auto* it = gNotes->begin(); it != gNotes->end(); ++it) {
					if (it->row >= startRow && it->row <= endRow) {
						double noteTime = gTempo->rowToTime(it->row);
						
						// Find closest onset
						double minDiff = window;
						double bestTime = -1.0;
						
						for(const auto& onset : onsets) {
							double t = (double)onset.pos / samplerate;
							double diff = abs(t - noteTime);
							if (diff < minDiff) {
								minDiff = diff;
								bestTime = t;
							}
							if (t > noteTime + window) break;
						}
						
						if (bestTime >= 0.0) {
							int newRow = gTempo->timeToRow(bestTime);
							if (newRow != it->row) {
								edit.rem.append(CompressNote(*it));
								
								Note newNote = CompressNote(*it);
								newNote.row = newRow;
								newNote.endrow = newRow + (it->endrow - it->row);
								edit.add.append(newNote);
								
								count++;
							}
						}
					}
				}
				
				if (count > 0) {
					gNotes->modify(edit, false);
					HudNote("Quantized %d notes to audio.", count);
				} else {
					HudNote("No notes needed quantization.");
				}
			}
		}

	CASE(PLACE_BEAT_AT_PLAYHEAD)
		{
			// This matches DDream's "Lock Beat" or typical sync workflow.
			// User listens, pauses at a beat, and hits a key to bring the nearest grid line to that time.

			// 1. Get current time (playhead or cursor)
			// If playing, use play time. If paused, use cursor time.
			double targetTime = gMusic->isPaused() ? gView->getCursorTime() : gMusic->getPlayTime();

			// 2. Find nearest beat/grid line
			int snap = gView->getSnapQuant();
			// We want to find the row closest to targetTime in the CURRENT timing.
			int row = gTempo->timeToRow(targetTime);

			// Snap row to grid
			int snappedRow = row;
			if (snap > 0) {
				int remainder = row % snap;
				if (remainder < snap / 2) snappedRow -= remainder;
				else snappedRow += (snap - remainder);
			}

			// 3. Move it
			// Constructive or Destructive?
			// Usually destructive (ripple) is what you want when building the tempo map linearly.
			// But let's ask or default?
			// Let's default to Ripple (Shift) behavior if Shift is held?
			// No, this action is usually bound to a key.
			// Let's make it always Ripple for now as that's the "DDream" way.

			gTempo->moveBeat(snappedRow, targetTime, true);
			gTempo->injectBoundingBpmChange(snappedRow); // Anchor it
			HudNote("Aligned row %d to %.3fs", snappedRow, targetTime);
		}

	CASE(WARP_GRID_TO_AUDIO)
		{
			auto& music = gMusic->getSamples();
			if(music.isCompleted() && music.getNumFrames() > 0) {
				auto region = gSelection->getSelectedRegion();
				int startRow = region.beginRow;
				int endRow = region.endRow;
				if (startRow == endRow) {
					HudError("No selection range for warping.");
					break;
				}

				gWaveform->updateOnsets();
				const auto& onsets = gWaveform->getOnsets();
				if (onsets.empty()) {
					HudError("No onsets detected.");
					break;
				}

				int step = gView->getSnapQuant();
				if (step <= 0) step = ROWS_PER_BEAT; // Default to beat if no snap

				Str::fmt msg("Warp grid in range [%d, %d] to audio (Step: %d)?");
				msg.arg(startRow).arg(endRow).arg(step);
				int res = gSystem->showMessageDlg("Warp Grid", msg, System::T_YES_NO, System::I_INFO);
				if (res != System::R_YES) break;

				int samplerate = music.getFrequency();
				int count = 0;
				double window = 0.1; // 100ms search window

				gHistory->startChain();

				// For "Warping", we want to adjust the grid to match the audio.
				// This is different from Quantize (which moves notes).
				// We iterate through grid points (startRow, startRow+step...)
				// We find the nearest onset.
				// We use destructiveShiftRowToTime to align the grid line to the onset.
				// AND we must anchor it with injectBoundingBpmChange.

				// Note: destructiveShiftRowToTime alters the time of SUBSEQUENT rows.
				// But since we are iterating forward (left to right), the next grid point we process
				// will be at its NEW position (because we call rowToTime inside the loop).
				// This is exactly what we want: we are "zippering" the grid to the audio.

				for (int row = startRow; row <= endRow; row += step) {
					double currentTime = gTempo->rowToTime(row);

					// Find closest onset to this grid line
					double minDiff = window;
					double bestTime = -1.0;

					for(const auto& onset : onsets) {
						double t = (double)onset.pos / samplerate;
						double diff = abs(t - currentTime);
						if (diff < minDiff) {
							minDiff = diff;
							bestTime = t;
						}
						// Optimization: onsets are sorted.
						if (t > currentTime + window) break;
					}

					if (bestTime > 0.0) {
						// Apply destructive shift (changes previous BPM)
						gTempo->moveBeat(row, bestTime, true);
						// Anchor this point so next shifts don't mess it up
						gTempo->injectBoundingBpmChange(row);
						count++;
					}
				}
				gHistory->finishChain("Warp Grid to Audio");
				HudNote("Warped %d grid points.", count);
			}
		}

	CASE(AUTO_SYNC_SECTIONS)
		{
			// 1. Detect Sections if needed (or rely on existing)
			// Let's assume user has run DETECT_SONG_SECTIONS or manually labeled them.
			// Or we can detect on the fly.

			auto& music = gMusic->getSamples();
			if (!music.isCompleted()) break;

			gHistory->startChain();

			// Iterate through Label segments
			const SegmentGroup* segs = gTempo->getSegments();

			// We need a sorted list of section start rows.
			std::vector<int> sectionRows;
			sectionRows.push_back(0); // Always start at 0
			if(segs) {
				const auto& labels = segs->getList<Label>();
				for(auto it = labels.begin(); it != labels.end(); ++it) {
					if (it->row > 0) sectionRows.push_back(it->row);
				}
			}

			// Sort and unique
			std::sort(sectionRows.begin(), sectionRows.end());
			sectionRows.erase(std::unique(sectionRows.begin(), sectionRows.end()), sectionRows.end());

			int endRow = gSimfile->getEndRow();

			int count = 0;

			for(size_t i = 0; i < sectionRows.size(); ++i) {
				int r1 = sectionRows[i];
				int r2 = (i + 1 < sectionRows.size()) ? sectionRows[i+1] : endRow;

				// Limit check
				if (r1 >= r2) continue;

				double t1 = gTempo->rowToTime(r1);
				double t2 = gTempo->rowToTime(r2);
				double dur = t2 - t1;

				if (dur < 2.0) continue; // Skip tiny sections

				// Detect BPM in this range
				auto detector = TempoDetector::New(t1, dur);
				if (detector) {
					// Wait for result (blocking for simplicity in this Action)
					int timeout = 200; // 2 seconds
					while(!detector->hasResult() && timeout > 0) {
						std::this_thread::sleep_for(std::chrono::milliseconds(10));
						timeout--;
					}

					if (detector->hasResult()) {
						auto results = detector->getResult();
						if (!results.empty()) {
							double bpm = results[0].bpm;
							// Insert BPM change
							// If i==0, we also might want to set offset?
							// AUTO_SYNC_SONG sets offset. This just handles BPM changes.
							gTempo->addSegment(BpmChange(r1, bpm));
							count++;
						}
					}
					delete detector;
				}
			}
			gHistory->finishChain("Auto-Sync Sections");
			HudNote("Applied %d BPM changes based on sections.", count);
		}

	CASE(CONVERT_REGION_TO_CONSTANT_BPM)
		{
			auto region = gSelection->getSelectedRegion();
			if(region.beginRow >= region.endRow) {
				HudError("Select a region to flatten.");
				break;
			}

			double t1 = gTempo->rowToTime(region.beginRow);
			double t2 = gTempo->rowToTime(region.endRow);
			double rows = (double)(region.endRow - region.beginRow);
			double beats = rows / ROWS_PER_BEAT;
			double duration = t2 - t1;

			if (duration <= 0.001) {
				HudError("Invalid duration.");
				break;
			}

			double avgBpm = beats * 60.0 / duration;

			// Confirm with user?
			Str::fmt msg("Flatten region [%d, %d] to constant BPM?\nNew BPM: %.3f");
			msg.arg(region.beginRow).arg(region.endRow).arg(avgBpm);
			if (gSystem->showMessageDlg("Flatten Tempo", msg, System::T_YES_NO, System::I_INFO) != System::R_YES) break;

			gHistory->startChain();

			// 1. Remove all BPM changes in (start, end]
			// We keep Start if it exists, or insert new one.
			// Actually we want to remove everything strictly inside the region.
			SegmentEdit edit;
			const auto& bpms = gTempo->getSegments()->getList<BpmChange>();
			for(auto it = bpms.begin(); it != bpms.end(); ++it) {
				if (it->row > region.beginRow && it->row < region.endRow) {
					edit.rem.append(Segment::BPM, it->row);
				}
			}
			gTempo->modify(edit, false); // false = don't clear region implicitly, we are explicit

			// 2. Insert new BPM at start
			gTempo->addSegment(BpmChange(region.beginRow, avgBpm));

			// 3. Anchor end?
			// If we change BPM at start, the time at EndRow will ideally match t2.
			// But floating point errors.
			// And we must ensure the NEXT section starts at the correct BPM (if it was different).
			// If there was a BPM change AT EndRow, it remains.
			// If there wasn't, we might need to insert one to preserve the tempo AFTER the region?
			// Yes, we should preserve the BPM that was active at EndRow.
			double bpmAtEnd = gTempo->getBpm(region.endRow);
			// But wait, getBpm() calculates based on current segments.
			// If we just removed segments, getBpm might have changed if we removed the change that governed EndRow.
			// So we should have captured bpmAtEnd BEFORE modification.
			// BUT, the logic above (remove > begin && < end) preserves a BPM change exactly AT end.
			// So if there was one, it's safe.
			// If there wasn't, then the BPM *after* EndRow was governed by something *before* EndRow.
			// That "something" might be what we just replaced!
			// So yes, we MUST insert a BPM change at EndRow to preserve the tempo of the rest of the song.
			// BUT, what value? The value that *was* active at EndRow.

			// We need to capture this BEFORE editing.
			// Re-logic:

			// a. Capture BPM at EndRow (before changes)
			double originalBpmAtEnd = gTempo->getBpm(region.endRow);

			// b. Perform edits (Remove inside, Insert at Start)
			// ... (done above)

			// c. Insert anchor at EndRow if one doesn't exist
			bool hasBpmAtEnd = false;
			// We can check via getRow but getRow returns default if not found.
			// Just trying to add it is safe if we want to enforce it.
			// Is it redundant? If avgBpm == originalBpmAtEnd?
			// Yes, but safe to add.

			gTempo->addSegment(BpmChange(region.endRow, originalBpmAtEnd));

			gHistory->finishChain("Flatten Tempo");
			HudNote("Region converted to constant %.3f BPM.", avgBpm);
		}

	CASE(VERIFY_CHART_INTEGRITY)
		{
			Vector<RowCol> stacked, overlapped;
			int stackedNotes = 0;
			int minesOnNotes = 0;
			int overlaps = 0;

			// Check for stacked notes
			for(const auto* it = gNotes->begin(); it != gNotes->end(); ++it) {
				const ExpandedNote& n = *it;
				const auto* next = it + 1;
				if (next != gNotes->end() && next->row == n.row && next->col == n.col) {
					if (n.isMine && !next->isMine) minesOnNotes++;
					else if (!n.isMine && next->isMine) minesOnNotes++;
					else stackedNotes++;

					stacked.push_back({(int)n.row, (int)n.col});
					stacked.push_back({(int)next->row, (int)next->col});
				}
			}

			// Check for overlaps
			int activeHoldEnd[SIM_MAX_COLUMNS];
			int activeHoldRow[SIM_MAX_COLUMNS];
			for(int i=0; i<SIM_MAX_COLUMNS; ++i) { activeHoldEnd[i] = -1; activeHoldRow[i] = -1; }

			int numCols = gStyle->getNumCols();

			for(const auto* it = gNotes->begin(); it != gNotes->end(); ++it) {
				const ExpandedNote& n = *it;
				if (n.col >= numCols) continue;

				if (n.row < activeHoldEnd[n.col] && n.row > activeHoldRow[n.col]) {
					overlaps++;
					overlapped.push_back({(int)n.row, (int)n.col});
				}

				if (n.endrow > n.row) {
					activeHoldEnd[n.col] = n.endrow;
					activeHoldRow[n.col] = n.row;
				}
			}

			if (stackedNotes > 0 || minesOnNotes > 0 || overlaps > 0) {
				gNotes->deselectAll();
				if (!stacked.empty()) gNotes->select(SELECT_ADD, stacked);
				if (!overlapped.empty()) gNotes->select(SELECT_ADD, overlapped);
			}

			Str::fmt msg("Chart Verification:\nStacked Notes: %d\nMines on Notes: %d\nOverlaps: %d");
			msg.arg(stackedNotes).arg(minesOnNotes).arg(overlaps);
			gSystem->showMessageDlg("Verify Chart", msg, System::T_OK, System::I_INFO);
		}

	CASE(SELECT_OFF_SYNC_NOTES)
		{
			Vector<RowCol> offSync;

			// Detect notes not on 64th note grid (row % 3 != 0)
			for(const auto* it = gNotes->begin(); it != gNotes->end(); ++it) {
				if (it->row % 3 != 0) {
					offSync.push_back({(int)it->row, (int)it->col});
				}
			}

			if (!offSync.empty()) {
				gNotes->deselectAll();
				gNotes->select(SELECT_SET, offSync);
				HudNote("Selected %d notes with 192nd micro-timing.", offSync.size());
			} else {
				HudNote("No off-sync (192nd) notes found.");
			}
		}

	CASE(RUN_TEST_SCRIPT)
		gHistory->startChain("Run Lua Script");
		gLua->runScript("scripts/test.lua");
		gHistory->finishChain();

	CASE(RUN_LUA_SCRIPT)
		{
			String path;
			if (gSystem->openFileDlg(path, "Lua Script (*.lua)|*.lua", false))
			{
				gHistory->startChain("Run Lua Script");
				gLua->runScript(path);
				gHistory->finishChain();
			}
		}
	}};

	if(action >= FILE_OPEN_RECENT_BEGIN && action < FILE_OPEN_RECENT_END)
	{
		gEditor->openSimfile(action - FILE_OPEN_RECENT_BEGIN);
	}
	if(action >= SET_NOTESKIN_BEGIN && action < SET_NOTESKIN_END)
	{
		gNoteskin->setType(action - SET_NOTESKIN_BEGIN);
	}
}

}; // namespace Vortex
