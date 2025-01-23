arguments:

for making indexes:

	program: make_si

		--config filename
		--type {psib, psif, psip, psim, bisb, bsif}
		--block_size=n
		--max_blocks=n
		--max_freq=n
		--psip_radius_uniform=d
		--psip_radius_pdf=filename
		--psip_radius_uniform_pdf=-12 -6 -3 0 3 6 12
		--noise_ration=percentage
		--false_positive_rate=d
		--verbose
		--secrets=secret_1 "secret phrase 2" ... secret_n

for making query sets:

	--secrets=secret1 secret2 ... secretn
	--verbose
	--obfuscations=0,1,...

for querying secure database:

	--verbose
	--db="base_path"
		NOTE: just a set of indexes
		NOTE: in base path, a config file will specify parameters like wether these secure indexes
			  used stemming and which stop words they used
	--timelag
	--stop_words=filename	NOTE: ideally use same stop word list as was used in making the secure indexes
	--porter_stemming		NOTE: only use stemming if the secure index also used stemming
	--query="this is phrase 1" "this is phrase 2" keyword1 keyword2
	--query_set=query_set_filename

	ONE OF:
	--bm25 k1=1.2 b=0.75
	--min_pair_distances
	--min_pair_score
	--frequency				NOTE: if multiple terms per query, show results for each
	--locations				NOTE: if multiple terms per query, show results for each


TO-DO:
	implement	PsiMin
				PsiPost
				FUTURE WORK: BsiFreq
				sensible combos
				use getHeader

test params:

	FUTURE WORK: unigram, bigram, trigram, ..., k-gram (best for frequency-only structures like PsiFreq: space compact and need extra assurance of larger n-grams
	to avoid false positives)

	query obfuscation: how is MAP/lagtime/filesize/memory consumption affected by (a) secret concats added, (b) noise terms added

	how does loading factor affect output? (MAP, file size, ...)

other notes:

	if a document has multiple SecureIndex files -- what to do in response to a query?
		* use the most accurate secure index for relevancy (which may be difficult to define, except in cases where the same secure index type is used
			for each one and the only differences are params like words per block)?
		* use the average relevancy?
			or use a weighted linear combination of them?
		* use the min relevancy?
		* use the max relevancy?
		* use the fastest?
		--- no single right answer: this is a trade-off question

	FUTURE WORK: provide complete interface for: adding secure indexes, removing secure indexes, checking that secure indexes are up to date, making
		sure the secure index in a db still has a legitimate/valid reference (e.g., does the reference still exist?)
			*** all of this stuff is easier if you assume the encrypted docs (which are being referred to in the secure index) are also in the database,
				but it may be the case that you wish to use separate remote databases for the secure indexes and the encrypted docs for security reasons.

	FUTURE WORK: non-local block 0: make block 0 in block-based secure indexes a "nonlocal" (things in it imply nothing about position) container. so,
		keywords/phrases (e.g., email:queelius@gmail.com) can be inserted into it, etc. on the query end, inform the
		query processor that this such "terms" should not be modified, e.g., do not break them down into biwords, do
		not stop-word them, do not stem them, etc.
		FUTURE WORK: make an "embeddable" schema to tell document processor how to handle certain tags, e.g.,
			if comes across tag <ignore>...</ignore>, do not add the "..." to secure index
			if comes across tag <soundex>...</soundex>, add soundex of term
			if comes across tag <variations>this is variation 1; this is also a variation; ...</variations>
				example for synonym use: <variations>cat; lion; tiger</variations>
					NOTE: using a natural language processor, stuff like this can be automatically done in a preprocessing step
					--- however, this will inflate secure index size
					--- another solution is to to preprocess the queries (query expansion)
			if comes across tag <exact>this whole exact phrase is inserted into it</exact>, then at that exact phrase (not using biword model) to index,
				and no stemming, no stopword removal, etc is done on it even if stemming/stopword removal is specified
			if comes across tag <keyword>term1; <exaxt>this is an exact keyword phrase</exact>; hello world</keyword>
				keyword terms are "non-local", they don't factor into proximity scores, only term weighting.
					they can be given a manual weight: <keyword>term1<weight>1000</weight>; ...</keyword>

	Experiment:

		- loadExperiment(Experiment stream)

		- run(numTrials)

		- stats:

			- how long does it take to construct Secure Indexes of Corpus?
			- look into Valgrind to examine how much memory is being used by program
				http://valgrind.org/
				*** often it should be close to the file size (for some of the indexes), so maybe just focus on that instead
			- mean average precision
			- average (lag) time to service a query
				-> average time to service a query normalized by number of secure indexes to process
				-> record properties like how many terms per query, how many words per term, etc.
			- Mean average precision
			- recall
			- precision

	ExperimentBuilder -> Experiment

		setRelevancyScoring(type)

			- Proximity scoring: { none, minimum pair-wise distance }
			- Keyword/term weighting: { none, bm25 }

		setReferenceIndex - type of nonsecure index to use as reference { InvertedIndex }

		setSecureIndexBuilder(ISecureIndexBuilder)

			- ISecureIndexBuilder is an interface implemented for:
				PsiBlock, PsiFreq, PsiPost, PsiMinPair, BsiBlock
				It is processed by a ISecureIndexBuilder over a Corpus

		setCorpus(Corpus)

		setQuerySet(QuerySet)

	CorpusBuilder -> Corpus

		- setOutputDirectory(path)

		- addInputDirectory(path, label=none)

			* different "classes" of corpus/directories, e.g., public domain books, medical texts, genomic data

		- setProperties(properties)

			* properties include: repeats (by filename) = false (in final build), recursive = true, file pattern = *.*, number to get n,
				distribution: "file properties" -> probability (discrete distribution)

		- build()

			* from the set of input directories (and the global properties -- allow local per directory properties?), 
				add filenames to a a list

		- getStatistics() -> CorpusStatistics: total words

		- write(outstream)
		- read(instream)

		- getCorpus() -> Corpus [list of filenames]

		- getOutputDirectory()
		- getInputDirectories()
		- getProperties()

	QuerySetBuilder -> QuerySet

		- how many words per query term?
		- how many query terms?
		- how much obfucation?
		- how many secret concats?

		- build()



TODO: find out if InvertedIndex is finding stuff correctly; find out if others are also!



verification:

	use a simple doc to check that PsiBlock and BsiBlock result in same (approximately) measurements for equal sized words per block, etc.





	in the is to am im a an and are as for of no by was his at which with this on her he be that then than him from or to but where not were
it have had been who has we may can if i its their these theyre youre your




!!!!!!!!!!!!!!!!!!!!! Lorem Ipsum generator. !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!




\b(in|the|is|to|am|im|a|an|and|are|as|for|of|no|by|was|his|at|which|with|this|on|her|he|be|that|then|than|him|from|or|to|but|where|not|were)\b