/*
 * Copyright 2009 Haiku, Inc.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Ankur Sethi (get.me.ankur@gmail.com)
 */

#include "BeaconIndex.h"

#include <Node.h>
#include <NodeInfo.h>
#include <Path.h>

#include <iostream>
using namespace std ;

using namespace lucene::util ;
using namespace lucene::document ;

BeaconIndex::BeaconIndex(BDirectory *dir)
	: fStatus(B_NO_INIT)
{
	SetTo(dir) ;
}


BeaconIndex::~BeaconIndex()
{
	Close() ;
}


status_t
BeaconIndex::InitCheck()
{
	return fStatus ;
}


status_t
BeaconIndex::SetTo(BDirectory *dir)
{
	if (InitCheck() == B_OK) {
		Close() ;
		delete fIndexWriter ;
	}

	fIndexWriter = OpenIndex(dir) ;
	if (fIndexWriter == NULL)
		fStatus = B_ERROR ;
	else {
		fStatus = B_OK ;
		BPath indexPath(dir) ;
		indexPath.Append("index") ;
		fIndexDirectory.SetTo(indexPath.Path()) ;
	}

	return fStatus ;
}


IndexWriter*
BeaconIndex::OpenIndex(BDirectory *dir)
{
	IndexWriter* indexWriter = NULL ;
	BPath path(dir) ;
	path.Append("index") ;
	
	if(create_directory(path.Path(), 0777) < B_OK)
		return NULL ;
	
	if (IndexReader::indexExists(path.Path())) {
		try {
			indexWriter = new IndexWriter(path.Path(), &fStandardAnalyzer, 
				false) ;
		} catch (CLuceneError error) {
			if (error.number() == CL_ERR_IO) {
				BEntry entry ;
				BDirectory indexDirectory(path.Path()) ;
				while (indexDirectory.GetNextEntry(&entry, false) == B_OK)
					entry.Remove() ;

				indexWriter = OpenIndex(dir) ;
			} else
				return NULL ;
		}
	} else
		indexWriter = new IndexWriter(path.Path(), &fStandardAnalyzer,
			true) ;
	
	return indexWriter ;
}


void
BeaconIndex::Close()
{
	fIndexWriter->optimize() ;
	fIndexWriter->close() ;
	fStatus = B_NO_INIT ;
}


bool
BeaconIndex::TranslatorAvailable(entry_ref *ref)
{
	if (!ref)
		return false ;

	// For now, just return true if the file is a plain text file.
	// Will change in the future when we have more translators.
	BNode node(ref) ;
	BNodeInfo nodeInfo(&node) ;

	char MIMEString[B_MIME_TYPE_LENGTH] ;
	nodeInfo.GetType(MIMEString) ;
	if (!strcmp(MIMEString, "text/plain"))
		return true ;
	
	return false ;
}


status_t
BeaconIndex::AddDocument(entry_ref *ref)
{
	if (fStatus != B_OK)
		return fStatus ;

	if (!ref || !TranslatorAvailable(ref) || Excluded(ref))
		return B_BAD_VALUE ;

	BPath path(ref) ;
	FileReader *fileReader = new FileReader(path.Path(), "ASCII") ;

	Document *doc = new Document ;
	doc->add(*(new Field("contents", fileReader, 
		Field::STORE_NO | Field::INDEX_TOKENIZED))) ;
	doc->add(*(new Field ("path", path.Path(),
		Field::STORE_YES | Field::INDEX_UNTOKENIZED))) ;

	try {
		fIndexWriter->addDocument(doc) ;
	} catch (CLuceneError) {
		return B_ERROR ;
	}

	return B_OK ;
}


bool
BeaconIndex :: Excluded(entry_ref *e_ref)
{
	BEntry entry(e_ref) ;
	if (fIndexDirectory.Contains(&entry))
		return true ;
	
	return false ;
}
