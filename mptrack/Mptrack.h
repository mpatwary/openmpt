/*
 * MPTrack.h
 * ---------
 * Purpose: OpenMPT core application class.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include "BuildSettings.h"

#include "resource.h"       // main symbols
#include "Settings.h"
#include "MPTrackUtil.h"
#include "Reporting.h"
#include "../soundlib/MIDIMacros.h"
#include "../soundlib/modcommand.h"
#include "../common/ComponentManager.h"
#include "../common/mptMutex.h"
#include "../common/mptRandom.h"

OPENMPT_NAMESPACE_BEGIN

class CModDoc;
class CModDocTemplate;
class CVstPluginManager;
namespace SoundDevice {
class Manager;
} // namespace SoundDevice
class CDLSBank;
class DebugSettings;
class TrackerSettings;
class ComponentManagerSettings;
namespace mpt { namespace Wine {
class VersionContext;
class Context;
} } // namespace mpt::Wine
#ifdef MPT_WITH_GDIPLUS
class GdiplusRAII;
#endif // MPT_WITH_GDIPLUS


/////////////////////////////////////////////////////////////////////////////
// 16-colors DIB
struct MODPLUGDIB
{
	BITMAPINFOHEADER bmiHeader;
	RGBQUAD bmiColors[16];
	LPBYTE lpDibBits;
};


/////////////////////////////////////////////////////////////////////////////
// Midi Library

struct MIDILIBSTRUCT
{
	mpt::PathString MidiMap[128*2];	// 128 instruments + 128 percussions
};


//////////////////////////////////////////////////////////////////////////
// Dragon Droppings

enum DragonDropType
{
	DRAGONDROP_NOTHING=0,	// |------< Drop Type >-------------|--< dwDropItem >---|--< lDropParam >---|
	DRAGONDROP_DLS,			// | Instrument from a DLS bank     |     DLS Bank #    |   DLS Instrument  |
	DRAGONDROP_SAMPLE,		// | Sample from a song             |     Sample #      |       NULL        |
	DRAGONDROP_INSTRUMENT,	// | Instrument from a song         |     Instrument #  |       NULL        |
	DRAGONDROP_SOUNDFILE,	// | File from instrument library   |        ?          |     File Name     |
	DRAGONDROP_MIDIINSTR,	// | File from midi library         | Midi Program/Perc |     File Name     |
	DRAGONDROP_PATTERN,		// | Pattern from a song            |      Pattern #    |       NULL        |
	DRAGONDROP_ORDER,		// | Pattern index in a song        |       Order #     |       NULL        |
	DRAGONDROP_SONG,		// | Song file (mod/s3m/xm/it)      |       0           |     File Name     |
	DRAGONDROP_SEQUENCE		// | Sequence (a set of orders)     |    Sequence #     |       NULL        |
};

struct DRAGONDROP
{
	CModDoc *pModDoc;
	DragonDropType dwDropType;
	DWORD dwDropItem;
	LPARAM lDropParam;

	mpt::PathString GetPath() const
	{
		const mpt::PathString *const pPath = reinterpret_cast<const mpt::PathString *>(lDropParam);
		ASSERT(pPath);
		if(!pPath)
		{
			return mpt::PathString();
		}
		return *pPath;
	}
};


/////////////////////////////////////////////////////////////////////////////
// CTrackApp:
// See mptrack.cpp for the implementation of this class
//

class CMPTCommandLineInfo;

class CTrackApp: public CWinApp
{
	friend class CMainFrame;
// static data
protected:
	static MODTYPE m_nDefaultDocType;
	static MIDILIBSTRUCT midiLibrary;

public:
	static std::vector<CDLSBank *> gpDLSBanks;

protected:

	mpt::recursive_mutex_with_lock_count m_GlobalMutex;

	DWORD m_GuiThreadId = 0;

	std::unique_ptr<mpt::random_device> m_RD;
	std::unique_ptr<mpt::thread_safe_prng<mpt::best_prng> > m_BestPRNG;
	std::unique_ptr<mpt::thread_safe_prng<mpt::prng> > m_PRNG;

#ifdef MPT_WITH_GDIPLUS
	std::unique_ptr<GdiplusRAII> m_Gdiplus;
#endif // MPT_WITH_GDIPLUS

	std::shared_ptr<mpt::Wine::VersionContext> m_WineVersion;

	IniFileSettingsBackend *m_pSettingsIniFile;
	SettingsContainer *m_pSettings = nullptr;
	DebugSettings *m_pDebugSettings = nullptr;
	TrackerSettings *m_pTrackerSettings = nullptr;
	IniFileSettingsBackend *m_pSongSettingsIniFile = nullptr;
	SettingsContainer *m_pSongSettings = nullptr;
	ComponentManagerSettings *m_pComponentManagerSettings = nullptr;
	IniFileSettingsContainer *m_pPluginCache = nullptr;
	CModDocTemplate *m_pModTemplate = nullptr;
	CVstPluginManager *m_pPluginManager = nullptr;
	SoundDevice::Manager *m_pSoundDevicesManager = nullptr;
	mpt::PathString m_szExePath;
	mpt::PathString m_szConfigDirectory;
	mpt::PathString m_szConfigFileName;
	mpt::PathString m_szPluginCacheFileName;
	std::shared_ptr<mpt::Wine::Context> m_Wine;
	mpt::PathString m_WineWrapperDllName;
	// Default macro configuration
	MIDIMacroConfig m_MidiCfg;
	DWORD m_dwLastPluginIdleCall = 0;
	bool m_bPortableMode = false;

public:
	CTrackApp();

	void AddToRecentFileList(LPCTSTR lpszPathName) override;
	void AddToRecentFileList(const mpt::PathString &path);
	/// Removes item from MRU-list; most recent item has index zero.
	void RemoveMruItem(const size_t item);
	void RemoveMruItem(const mpt::PathString &path);

public:

	mpt::PathString GetAppDirPath() {return m_szExePath;} // Returns '\'-ended executable directory path.
	static MODTYPE GetDefaultDocType() { return m_nDefaultDocType; }
	static void SetDefaultDocType(MODTYPE n) { m_nDefaultDocType = n; }
	static MIDILIBSTRUCT &GetMidiLibrary() { return midiLibrary; }
	static BOOL ImportMidiConfig(const mpt::PathString &filename, BOOL bNoWarning=FALSE);
	static BOOL ExportMidiConfig(const mpt::PathString &filename);
	static BOOL ImportMidiConfig(SettingsContainer &file, bool forgetSettings = false);
	static BOOL ExportMidiConfig(SettingsContainer &file);
	static void LoadDefaultDLSBanks();
	static void SaveDefaultDLSBanks();
	static void RemoveDLSBank(UINT nBank);
	static bool AddDLSBank(const mpt::PathString &filename);
	static bool OpenURL(const char *url); // UTF8
	static bool OpenURL(const std::string &url); // UTF8
	static bool OpenURL(const CString &url);
	static bool OpenURL(const mpt::ustring &url);
	static bool OpenURL(const mpt::PathString &lpszURL);
	static bool OpenFile(const mpt::PathString &file) { return OpenURL(file); };
	static bool OpenDirectory(const mpt::PathString &directory) { return OpenURL(directory); };

	// Retrieve the user-supplied MIDI port name for a MIDI input or output port.
	mpt::ustring GetFriendlyMIDIPortName(const mpt::ustring &deviceName, bool isInputPort, bool addDeviceName = true);
	CString GetFriendlyMIDIPortName(const CString &deviceName, bool isInputPort, bool addDeviceName = true);

	int GetOpenDocumentCount() const;
	std::vector<CModDoc *> GetOpenDocuments() const;

public:
	inline mpt::recursive_mutex_with_lock_count & GetGlobalMutexRef() { return m_GlobalMutex; }
	bool InGuiThread() const { return GetCurrentThreadId() == m_GuiThreadId; }
	mpt::random_device & RandomDevice() { return *m_RD; }
	mpt::thread_safe_prng<mpt::best_prng> & BestPRNG() { return *m_BestPRNG; }
	mpt::thread_safe_prng<mpt::prng> & PRNG() { return *m_PRNG; }
	CModDocTemplate *GetModDocTemplate() const { return m_pModTemplate; }
	CVstPluginManager *GetPluginManager() const { return m_pPluginManager; }
	SoundDevice::Manager *GetSoundDevicesManager() const { return m_pSoundDevicesManager; }
	void GetDefaultMidiMacro(MIDIMacroConfig &cfg) const { cfg = m_MidiCfg; }
	void SetDefaultMidiMacro(const MIDIMacroConfig &cfg) { m_MidiCfg = cfg; }
	mpt::PathString GetConfigFileName() const { return m_szConfigFileName; }
	SettingsContainer & GetSettings()
	{
		ASSERT(m_pSettings);
		return *m_pSettings;
	}
	TrackerSettings & GetTrackerSettings()
	{
		ASSERT(m_pTrackerSettings);
		return *m_pTrackerSettings;
	}
	bool IsPortableMode() { return m_bPortableMode; }
	SettingsContainer & GetPluginCache()
	{
		ASSERT(m_pPluginCache);
		return *m_pPluginCache;
	}

	SettingsContainer & GetSongSettings()
	{
		ASSERT(m_pSongSettings);
		return *m_pSongSettings;
	}
	const mpt::PathString& GetSongSettingsFilename() const
	{
		return m_pSongSettingsIniFile->GetFilename();
	}

	void SetWineVersion(std::shared_ptr<mpt::Wine::VersionContext> wineVersion)
	{
		MPT_ASSERT_ALWAYS(mpt::Windows::IsWine());
		m_WineVersion = wineVersion;
	}
	std::shared_ptr<mpt::Wine::VersionContext> GetWineVersion() const
	{
		MPT_ASSERT_ALWAYS(mpt::Windows::IsWine());
		MPT_ASSERT_ALWAYS(m_WineVersion); // Verify initialization order. We should not should reach this until after Wine is detected.
		return m_WineVersion;
	}

	void SetWine(std::shared_ptr<mpt::Wine::Context> wine)
	{
		m_Wine = wine;
	}
	std::shared_ptr<mpt::Wine::Context> GetWine() const
	{
		return m_Wine;
	}

	void SetWineWrapperDllFilename(mpt::PathString filename)
	{
		m_WineWrapperDllName = filename;
	}
	mpt::PathString GetWineWrapperDllFilename() const
	{
		return m_WineWrapperDllName;
	}

	/// Returns path to config folder including trailing '\'.
	mpt::PathString GetConfigPath() const { return m_szConfigDirectory; }
	void SetupPaths(bool overridePortable);
	void CreatePaths();

	CString SuggestModernBuildText();
	bool CheckSystemSupport();

	// Relative / absolute paths conversion
	mpt::PathString AbsolutePathToRelative(const mpt::PathString &path) { return path.AbsolutePathToRelative(GetAppDirPath()); }
	mpt::PathString RelativePathToAbsolute(const mpt::PathString &path) { return path.RelativePathToAbsolute(GetAppDirPath()); }

	static void OpenModulesDialog(std::vector<mpt::PathString> &files, const mpt::PathString &overridePath = mpt::PathString());

public:
	// Get name of resampling mode. addTaps = true also adds the number of taps the filter uses.
	static const TCHAR *GetResamplingModeName(ResamplingMode mode, bool addTaps);

// Overrides
public:
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTrackApp)
public:
	BOOL InitInstance() override;
	BOOL InitInstanceEarly(CMPTCommandLineInfo &cmdInfo);
	BOOL InitInstanceLate(CMPTCommandLineInfo &cmdInfo);
	BOOL InitInstanceImpl(CMPTCommandLineInfo &cmdInfo);
	int Run() override;
	LRESULT ProcessWndProcException(CException* e, const MSG* pMsg) override;
	int ExitInstance() override;
	int ExitInstanceImpl();
	BOOL OnIdle(LONG lCount) override;
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CTrackApp)
	CModDoc *NewDocument(MODTYPE newType = MOD_TYPE_NONE);

	afx_msg void OnFileNew() { NewDocument(); }
	afx_msg void OnFileNewMOD() { NewDocument(MOD_TYPE_MOD); }
	afx_msg void OnFileNewS3M() { NewDocument(MOD_TYPE_S3M); }
	afx_msg void OnFileNewXM() { NewDocument(MOD_TYPE_XM); }
	afx_msg void OnFileNewIT() { NewDocument(MOD_TYPE_IT); }
	afx_msg void OnFileNewMPT() { NewDocument(MOD_TYPE_MPT); }

	afx_msg void OnFileOpen();
	afx_msg void OnAppAbout();

	afx_msg void OnFileCloseAll();
	afx_msg void OnUpdateAnyDocsOpen(CCmdUI *cmd);

	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	BOOL InitializeDXPlugins();
	BOOL UninitializeDXPlugins();


	bool MoveConfigFile(mpt::PathString sFileName, mpt::PathString sSubDir = mpt::PathString(), mpt::PathString sNewFileName = mpt::PathString());

};


extern CTrackApp theApp;


//////////////////////////////////////////////////////////////////
// More Bitmap Helpers

class CFastBitmap
{
protected:
	static const uint8 BLEND_OFFSET = 0x80;

	struct MODPLUGFASTDIB
	{
		BITMAPINFOHEADER bmiHeader;
		RGBQUAD bmiColors[256];
		std::vector<uint8> DibBits;
	};

	MODPLUGFASTDIB m_Dib;
	UINT m_nTextColor, m_nBkColor;
	MODPLUGDIB *m_pTextDib;
	uint8 m_nBlendOffset;
	uint8 m_n4BitPalette[16];
	uint8 m_nXShiftFactor;

public:
	CFastBitmap() {}

public:
	void Init(MODPLUGDIB *lpTextDib = nullptr);
	void Blit(HDC hdc, int x, int y, int cx, int cy);
	void Blit(HDC hdc, LPCRECT lprc) { Blit(hdc, lprc->left, lprc->top, lprc->right-lprc->left, lprc->bottom-lprc->top); }
	void SetTextColor(int nText, int nBk=-1) { m_nTextColor = nText; if (nBk >= 0) m_nBkColor = nBk; }
	void SetTextBkColor(UINT nBk) { m_nBkColor = nBk; }
	void SetColor(UINT nIndex, COLORREF cr);
	void SetAllColors(UINT nBaseIndex, UINT nColors, COLORREF *pcr);
	void TextBlt(int x, int y, int cx, int cy, int srcx, int srcy, MODPLUGDIB *lpdib = nullptr);
	void SetBlendMode(bool enable) { m_nBlendOffset = enable ? BLEND_OFFSET : 0; }
	bool GetBlendMode() const { return m_nBlendOffset != 0; }
	void SetBlendColor(COLORREF cr);
	void SetSize(int x, int y);
	int GetWidth() const { return m_Dib.bmiHeader.biWidth; }
};


///////////////////////////////////////////////////
// 4-bit DIB Drawing functions
void DibBlt(HDC hdc, int x, int y, int sizex, int sizey, int srcx, int srcy, MODPLUGDIB *lpdib);
MODPLUGDIB *LoadDib(LPCTSTR lpszName);
RGBQUAD rgb2quad(COLORREF c);

// Other bitmap functions
void DrawButtonRect(HDC hdc, LPRECT lpRect, LPCSTR lpszText=NULL, BOOL bDisabled=FALSE, BOOL bPushed=FALSE, DWORD dwFlags=(DT_CENTER|DT_VCENTER));
void DrawButtonRect(HDC hdc, LPRECT lpRect, LPCWSTR lpszText=NULL, BOOL bDisabled=FALSE, BOOL bPushed=FALSE, DWORD dwFlags=(DT_CENTER|DT_VCENTER));

// Misc functions
UINT MsgBox(UINT nStringID, CWnd *p=NULL, LPCSTR lpszTitle=NULL, UINT n=MB_OK);
void ErrorBox(UINT nStringID, CWnd*p=NULL);

// Helper function declarations.
struct SNDMIXPLUGIN;
class IMixPlugin;
void AddPluginNamesToCombobox(CComboBox& CBox, const SNDMIXPLUGIN *plugarray, const bool librarynames = false);
void AddPluginParameternamesToCombobox(CComboBox& CBox, SNDMIXPLUGIN& plugarray);
void AddPluginParameternamesToCombobox(CComboBox& CBox, IMixPlugin& plug);

// Append note names in range [noteStart, noteEnd] to given combobox. Index starts from 0.
void AppendNotesToControl(CComboBox& combobox, ModCommand::NOTE noteStart, ModCommand::NOTE noteEnd);

// Append note names to combo box.
// If nInstr is given, instrument-specific note names are used instead of default note names.
// A custom note range may also be specified using the noteStart and noteEnd parameters.
// If they are left out, only notes that are available in the module type, plus any supported "special notes" are added.
void AppendNotesToControlEx(CComboBox& combobox, const CSoundFile &sndFile, INSTRUMENTINDEX nInstr = MAX_INSTRUMENTS, ModCommand::NOTE noteStart = 0, ModCommand::NOTE noteEnd = 0);

// Get window text (e.g. edit box content) as a CString
CString GetWindowTextString(const CWnd &wnd);

// Get window text (e.g. edit box content) as a unicode string
mpt::ustring GetWindowTextUnicode(const CWnd &wnd);

///////////////////////////////////////////////////
// Tables

extern const TCHAR *szSpecialNoteNamesMPT[];
extern const TCHAR *szSpecialNoteShortDesc[];
extern const char *szHexChar;

// Defined in load_mid.cpp
extern const char *szMidiProgramNames[128];
extern const char *szMidiPercussionNames[61];	// notes 25..85
extern const char *szMidiGroupNames[17];		// 16 groups + Percussions

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.


OPENMPT_NAMESPACE_END
