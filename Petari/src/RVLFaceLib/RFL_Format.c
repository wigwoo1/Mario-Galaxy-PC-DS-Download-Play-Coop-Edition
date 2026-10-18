#include "RVLFaceLibInternal.h"
#include <mem.h>

#define MAGIC_OFFICIAL_DB 'RNOD'
#define MAGIC_HIDDEN_DB 'RNHD'

#define TABLE_DATA_STEP 1250
// TODO: How is this size calculated?
#define TEMP_BUFFER_SIZE (TABLE_DATA_STEP * sizeof(RFLiHiddenCharData) + 0x30E0)

void RFLiAllocateDB(RFLiDatabase* db) {
    memset((u8*)db + sizeof(db->identifier), 0, sizeof(RFLiDatabase));
}

static void RFLiClearTableData(RFLiTableData* data) {
    memset(data, 0, sizeof(RFLiTableData));
    data->next = -1;
    data->prev = -1;
}

void RFLiClearDBBuffer(void) {
    RFLiDatabase* db;
    RFLiHiddenDB* header;
    int i;

    db = RFLiGetDBManager()->database;
    header = &db->hidden;

    // @bug Four-byte buffer overrun
    RFLiAllocateDB(db);
    db->identifier = MAGIC_OFFICIAL_DB;
    db->isolation = TRUE;

    header->identifier = MAGIC_HIDDEN_DB;
    header->head = -1;
    header->tail = -1;

    for (i = 0; i < RFLi_HDB_DATA_MAX; i++) {
        RFLiClearTableData(&header->data[i]);
    }
}

/*
 * Create an empty FaceLib database when the NAND file is missing. Keep this
 * operation asynchronous like the other FaceLib file operations: the header
 * CRC and full database write are performed by RFLiSaveDatabaseAsync, then
 * the file is closed before the caller's continuation runs.
 *
 * Do not use this path to overwrite a database that failed validation. That
 * case is surfaced as Broken so the caller can report/repair it explicitly.
 */
static void formatCloseCallback_(void) {
    RFLiManager* manager = RFLiGetManager();
    RFLiHDBManager* hdb = RFLiGetHDBManager();
    RFLiCallback callback = hdb != NULL ? hdb->formatCb : NULL;

    if (RFLGetAsyncStatus() != RFLErrcode_Success && manager != NULL) {
        manager->brokenType |= 1 << RFLiFileBrokenType_DBBroken;
    }
    if (hdb != NULL) {
        hdb->formatCb = NULL;
    }
    if (callback != NULL) {
        callback();
    }
}

static void formatWriteCallback_(void) {
    RFLiCloseAsync(RFLiFileType_Database, formatCloseCallback_);
}

static void formatOpenCallback_(void) {
    RFLErrcode result;

    if (RFLGetAsyncStatus() != RFLErrcode_Success) {
        formatCloseCallback_();
        return;
    }

    result = RFLiSaveDatabaseAsync(formatWriteCallback_);
    if (result != RFLErrcode_Success && result != RFLErrcode_Busy) {
        RFLiGetManager()->lastErrCode = result;
        RFLiCloseAsync(RFLiFileType_Database, formatCloseCallback_);
    }
}

RFLErrcode RFLiFormatAsync(RFLiCallback callback) {
    RFLiManager* manager;
    RFLiDBManager* database;
    RFLiHDBManager* hdb;
    RFLErrcode result;

    if (!RFLAvailable()) {
        return RFLErrcode_NotAvailable;
    }
    if (RFLiIsWorking()) {
        return RFLErrcode_Busy;
    }

    manager = RFLiGetManager();
    database = RFLiGetDBManager();
    hdb = RFLiGetHDBManager();
    if (manager == NULL || database == NULL || database->database == NULL ||
        hdb == NULL) {
        return RFLErrcode_NotAvailable;
    }

    /* Formatting is safe only for a missing database, never a corrupt one. */
    if (!RFLiNotFoundError() || RFLiNeedRepairError()) {
        return RFLErrcode_Broken;
    }

    RFLiClearDBBuffer();
    manager->brokenType &= ~(1 << RFLiFileBrokenType_DBNotFound);
    hdb->formatCb = callback;

    result = RFLiOpenAsync(RFLiFileType_Database, 2, formatOpenCallback_);
    if (result != RFLErrcode_Success && result != RFLErrcode_Busy) {
        hdb->formatCb = NULL;
        manager->brokenType |= 1 << RFLiFileBrokenType_DBBroken;
    }
    return result;
}
