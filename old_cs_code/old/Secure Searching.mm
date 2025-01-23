<map version="1.0.1">
<!-- To view this file, download free mind mapping software FreeMind from http://freemind.sourceforge.net -->
<node CREATED="1393787340761" ID="ID_761294643" MODIFIED="1393787776774" TEXT="Secure Searching">
<node CREATED="1393787349989" ID="ID_1570308143" MODIFIED="1393819659697" POSITION="right" STYLE="fork" TEXT="Intro">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      Subset of homomorphic encryption. The only computation we permit on data is in the context of information retrieval. Fully homomorphic encryption is orders of magnitude slower, so we are solving a simpler problem.
    </p>
    <p>
      
    </p>
    <p>
      http://www.darpa.mil/Our_Work/I2O/Programs/PROgramming_Computation_on_EncryptEd_Data_(PROCEED).aspx
    </p>
  </body>
</html>
</richcontent>
<node CREATED="1393793240761" ID="ID_1082173521" MODIFIED="1393819659170" TEXT="Use cases">
<node CREATED="1393793243416" ID="ID_7362901" MODIFIED="1393819659170" TEXT="Email filtering -- does incoming email have &quot;urgent&quot; in subject?"/>
<node CREATED="1393793261449" ID="ID_70693036" MODIFIED="1393819659170" TEXT="Finding documents in a large encrypted document database"/>
</node>
</node>
<node CREATED="1393787355147" ID="ID_50686664" MODIFIED="1393883139156" POSITION="left" TEXT="Confidentiality">
<node CREATED="1393787433769" ID="ID_563810376" MODIFIED="1393793469046" TEXT="How is confidentiality accomplished?">
<node CREATED="1393787834233" ID="ID_1919644916" MODIFIED="1393787839510" TEXT="Obfusucation">
<node CREATED="1393787828617" ID="ID_156855074" MODIFIED="1393810218699" TEXT="Compression">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      Adding Compression to Block Addressing Inverted Indexes
    </p>
    <p>
      Compressed Bloom Filters (doesn't really obscure contents since bloom filter must be decompressed before using)
    </p>
    <p>
      
    </p>
    <p>
      Advantages:
    </p>
    <p>
      
    </p>
    <p>
      Easy to implement. Fast.
    </p>
    <p>
      
    </p>
    <p>
      Note that compression does not imply a size advantage over the other techniques, since the other techniques may also use compression. Here, we discuss compression in the context of making a quick and easy hidden symbol mapping as a weak form of encryption, e.g., for each word (or term) in the document, map that to a bit string s.t. the total length of the document using those bit strings is optimally compressed. Huffman encoding is the optimal compressor in this context (note: Arithmetic encoding is not suitable for this task since it shines when the WHOLE document is being compressed, whereas we want symbols to be compressed so that we may do searches on individual symbols.), so the idea would be to find such a symbol mapping and then not include the mapping on the server, hence, the symbol mapping is kept hidden.
    </p>
    <p>
      
    </p>
    <p>
      Disadvantages:
    </p>
    <ul>
      <li>
        Since you already have the symbol mapping, you can just query it locally, which can be treated as an index of the document.
      </li>
      <li>
        If optimal compression is not used per document, e.g., optimal compressor for the entire document collection is used, then can use the symbol mappings to query any document on the server, but...

        <ul>
          <li>
            Can be relatively easily broken using basic cryptoanalysis (probability theory)
          </li>
          <li>
            No longer nearly as space-efficient
          </li>
          <li>
            Obvious access patterns: frequency analysis on queries possible since each word in every document has the same encoding of 1's and 0's, so by looking for certain 1's and 0's you leak a lot of information

            <ul>
              <li>
                Combine with other info leaks, e.g., when submitting query q, user has high probability of following up with a plaintext search for stock information
              </li>
            </ul>
          </li>
        </ul>
      </li>
    </ul>
  </body>
</html>
</richcontent>
</node>
<node CREATED="1393807286457" ID="ID_493349288" MODIFIED="1393816488719" TEXT="Learning hash maps">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      Extremely Fast Text Feature Extraction for Classification and Indexing
    </p>
    <p>
      Building A Better Bloom Filter
    </p>
  </body>
</html>
</richcontent>
</node>
</node>
<node CREATED="1393793631609" ID="ID_1688760497" MODIFIED="1393793635222" TEXT="Encryption">
<node CREATED="1393793635929" ID="ID_1327069989" MODIFIED="1393812685661" TEXT="Symmetric">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      Practical Techniques for Searches on Encrypted Data
    </p>
  </body>
</html>
</richcontent>
</node>
<node CREATED="1393793638585" ID="ID_1829805974" MODIFIED="1393810015020" TEXT="Asymmetric">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      Public-key encryption with keyword search
    </p>
  </body>
</html>
</richcontent>
</node>
</node>
</node>
<node CREATED="1393787504617" FOLDED="true" ID="ID_298249005" MODIFIED="1393839434231" TEXT="Types of information leaks">
<node CREATED="1393787693017" ID="ID_828920673" MODIFIED="1393807815832" TEXT="Access patterns">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      Frequency analysis: if queries leak information (extreme case: queries sent in plaintext, non-extreme case: a query for keyword x always looks the same), then e.g., statistical inferences about how likely
    </p>
    <p>
      certain queries are can be used to map encrypted queries to plaintext queries.
    </p>
    <p>
      
    </p>
    <p>
      Correlation analysis: when submitting random looking query X, usually soon after looks up stock information.
    </p>
  </body>
</html>
</richcontent>
<node CREATED="1393787705337" ID="ID_1882287156" MODIFIED="1393807457957" TEXT="Oblivious RAM">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      Oblivious RAM Revisited
    </p>
  </body>
</html>
</richcontent>
</node>
<node CREATED="1393787714008" ID="ID_1306301096" MODIFIED="1393794384424" TEXT="Query randomization"/>
</node>
<node CREATED="1393787386377" ID="ID_379154981" MODIFIED="1393787392710" TEXT="Query confidentiality"/>
<node CREATED="1393787393673" ID="ID_1069260006" MODIFIED="1393787401926" TEXT="Document (data) confidentiality">
<node CREATED="1393787474153" ID="ID_870193885" MODIFIED="1393813775495" TEXT="Frequency analysis">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      If the same term in doc 1 and doc 2 have same representation in secure index, then this leaks a lot of information.
    </p>
  </body>
</html>
</richcontent>
<node CREATED="1393787538569" ID="ID_585199968" MODIFIED="1393808454669" TEXT="per document">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      If each mapping in a document (e.g., online index) maps a term to a specific pattern, then frequency analysis may be used to infer likely mappings.
    </p>
  </body>
</html>
</richcontent>
<node CREATED="1393787556009" ID="ID_1130781058" MODIFIED="1393787573478" TEXT="Same word in different document different"/>
<node CREATED="1393787752313" ID="ID_1233512987" MODIFIED="1393787756598" TEXT="In our case: per block?"/>
</node>
<node CREATED="1393787543322" ID="ID_1594009276" MODIFIED="1393808071619" TEXT="per collection">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      If each mapping for a term in any document in the collection maps to the same pattern, then frequency analysis may be used to infer likely mappings. This is more info leakage than per document mapping.
    </p>
    <p>
      
    </p>
    <p>
      So, a term in two different documents should have different representations, e.g., hash(term | secret | doc_id) instead of hash(term | secret), assuming the secret is the same in both cases and each document has a unique id.
    </p>
  </body>
</html>
</richcontent>
</node>
</node>
</node>
<node CREATED="1393787440105" ID="ID_667104380" MODIFIED="1393800029332" TEXT="Primary goal of encrypted search">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      Primary goal: preventing untrusted parties from inferring anything about the encrypted document beyond which documents are returned for a given encrypted query.
    </p>
    <p>
      
    </p>
    <p>
      Only accomplished by using encryption.
    </p>
    <p>
      
    </p>
    <p>
      Names for this? Many games were played to capture this idea.
    </p>
  </body>
</html>
</richcontent>
</node>
</node>
</node>
<node CREATED="1393787778377" ID="ID_163101904" MODIFIED="1393793990230" POSITION="right" TEXT="How do we map queries to documents?">
<node CREATED="1393787803897" ID="ID_789001815" MODIFIED="1393811535485" TEXT="Boolean keyword matching">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      Practical Techniques for Searches on Encrypted Data
    </p>
    <p>
      Public-key encryption with keyword search
    </p>
  </body>
</html>
</richcontent>
<node CREATED="1393787974409" ID="ID_684187686" MODIFIED="1393801686102" TEXT="Extensions">
<node CREATED="1393787989033" ID="ID_839815421" MODIFIED="1393809870687" TEXT="Conjunctive keyword search">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      Secure Conjunctive Keyword Search over Encrypted Data
    </p>
  </body>
</html>
</richcontent>
</node>
<node CREATED="1393787998185" ID="ID_1908260209" MODIFIED="1393823032551" TEXT="Approximate keyword matching">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      As the authors of &quot;Efficient Fuzzy Search in Large Text Collections&quot; point out, for Google, not returning enough results is not a problem. They're primary problem is finding ways to return fewer results so that users do not have to sift through too many irrelevant results. Thus, they are motivated to improve precision (the ratio of relevant documents returned to total documents returned). However, in vertical search--such as encrypted searching over an enterprise's store of encrypted documents--there is far more concern over not missing or overlooking relevant documents, since there may be so few to begin with. That is, recall (how many relevant documents were returned to total number of relevant documents) is just as important, if not more important, than precision.
    </p>
    <p>
      
    </p>
    <p>
      Elsewhere, we discuss &quot;relevancy scoring&quot;, which provides a more sophisticated approach in that the goal is to rank documents according to how relevant they are to a query as opposed to a simple &quot;relevant&quot; / &quot;irrelevant&quot; score found in Boolean keyword searching. But, a simple extension to Boolean keyword searching which improves the recall (at the expense of precision) is to do more tolerant matching on the keywords.
    </p>
  </body>
</html>
</richcontent>
<node CREATED="1393788008249" ID="ID_1087993059" MODIFIED="1393851414197" TEXT="Locality-sensitive hashing">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      Approximate Nearest Neighbors: Towards Removing the Curse of Dimensionality
    </p>
    <p>
      Locality-Sensitive Bloom Filter for Approximate Membership Query
    </p>
    <p>
      Distance-Sensitive Bloom Filters
    </p>
  </body>
</html>
</richcontent>
<node CREATED="1393788027369" ID="ID_1745218457" MODIFIED="1393788045878" TEXT="Stemming"/>
<node CREATED="1393788106393" ID="ID_1420683989" MODIFIED="1393788111862" TEXT="Phonetic algorithms"/>
<node CREATED="1393801534841" ID="ID_1404722753" MODIFIED="1393813559223" TEXT="Inexact measures on LSH">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      Distance-Sensitive Bloom Filters
    </p>
    <p>
      
    </p>
    <p>
      These, in the context of encrypted search, are not practical unless we are willing to give up some confidentiality guarantees.
    </p>
    <p>
      
    </p>
    <p>
      For instance, after applying a locality-sensitive hash to the terms in a document, we than usually apply a cryptographic hash to this to minimize information leakage. However, if we decide not to do this, then we can use distance metrics to see how close two Bloom filters are, e.g., Hamming distance.
    </p>
    <p>
      
    </p>
    <p>
      Trade-off between security and kinds of approximate matching.
    </p>
  </body>
</html>
</richcontent>
<node CREATED="1393788112937" ID="ID_647955561" MODIFIED="1393801528070" TEXT="Hamming metric"/>
</node>
</node>
<node CREATED="1393788016777" ID="ID_858598061" MODIFIED="1393809818180" TEXT="Edit distance">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      Fuzzy Keyword Search Over Encrypted Data In Cloud Computing
    </p>
    <p>
      
    </p>
    <p>
      Edit distance metrics are useful for approximate matching despite typographical errors or spelling variations, color vs colour.
    </p>
    <p>
      
    </p>
    <p>
      This can be done offline or online. Doing it online, in the context of encrypted search, doesn't seem practical, since a query would need to be expanded to all the k-edit variations. Doing it offline, where the secure index contains these errors, helps in the following way:
    </p>
    <p>
      
    </p>
    <p>
      insert k-edit error patterns to detect e.g.typographical errors up to k characters long -- storing only patterns makes problem space-tractable, but can still grow index size considerably.
    </p>
    <p>
      Then, to detect 1-edit errors, change a term in the query to match a k-edit error pattern. Far fewer patterns to explore, .. show math in doc.
    </p>
  </body>
</html>
</richcontent>
<node CREATED="1393802103449" ID="ID_847538744" MODIFIED="1393802108198" TEXT="Offline versus online"/>
</node>
<node CREATED="1393790690889" ID="ID_968502173" MODIFIED="1393809832112" TEXT="Wildcard matching">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      Fuzzy Keyword Search Over Encrypted Data In Cloud Computing
    </p>
    <p>
      
    </p>
    <p>
      Repurposes k-edit error patterns. Nothing more really needs to be done; inserting the patterns already enables this.
    </p>
  </body>
</html>
</richcontent>
<node CREATED="1393802493961" ID="ID_916978617" MODIFIED="1393802495702" TEXT="Use cases"/>
</node>
</node>
<node CREATED="1393788060873" ID="ID_1997317806" MODIFIED="1393809713152" TEXT="Exact phrase matching (word n-grams)">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      Phrase Search over Encrypted Data with Symmetric Encryption Scheme
    </p>
  </body>
</html>
</richcontent>
<node CREATED="1393801588169" ID="ID_1265701658" MODIFIED="1393801653160" TEXT="Biword model">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      No examples seen in the literature in context of encrypted searching.
    </p>
  </body>
</html>
</richcontent>
<node CREATED="1393801593753" ID="ID_1515678337" MODIFIED="1393801596134" TEXT="false positives"/>
<node CREATED="1393801662153" ID="ID_1406350177" MODIFIED="1393801665894" TEXT="extensions easy"/>
</node>
</node>
</node>
<node CREATED="1393793337897" ID="ID_1258788532" MODIFIED="1393793349750" TEXT="Likely use cases"/>
<node CREATED="1393793830073" ID="ID_521289764" MODIFIED="1393793831526" TEXT="Advantages">
<node CREATED="1393793832217" ID="ID_646036648" MODIFIED="1393793833478" TEXT="Simple"/>
</node>
<node CREATED="1393793837913" ID="ID_962404600" MODIFIED="1393793841014" TEXT="Disadvantages">
<node CREATED="1393793841673" ID="ID_741895740" MODIFIED="1393793944887" TEXT="Doesn&apos;t deliver very relevant results">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      Except in limited circumstances (e.g., knowing to look for &quot;urgent&quot; in a subject field on an incoming encrypted email), doesn't deliver good relevant results:
    </p>
    <p>
      
    </p>
    <p>
      1) Misses too many relevant documents -- poor recall
    </p>
    <p>
      2) Returns too many irrelevant documents -- poor precision
    </p>
  </body>
</html>
</richcontent>
<node CREATED="1393794265321" ID="ID_721953586" MODIFIED="1393794267158" TEXT="Recall"/>
<node CREATED="1393794267977" ID="ID_577489011" MODIFIED="1393794269509" TEXT="Precision"/>
</node>
</node>
</node>
<node CREATED="1393788152201" ID="ID_271134788" MODIFIED="1393816279944" TEXT="Relevancy -- &quot;context-aware&quot;">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      Confidentiality-Preserving Rank-Ordered Search
    </p>
    <p>
      
    </p>
    <p>
      Useful really only on a small-scale, e.g., scanning incoming subjects on emails for &quot;urgent&quot; since generally Boolean keyword search is inadequate once the collection of documents being searched over becomes very large, e.g., every document in the store may contain the provided keyword (or even conjuction of keywords), but the keywords may not be sufficiently close.
    </p>
    <p>
      
    </p>
    <p>
      So, in such cases, Boolean searching is not sufficient.
    </p>
    <p>
      
    </p>
  </body>
</html>
</richcontent>
<node CREATED="1393793792233" ID="ID_1852553261" MODIFIED="1393816340205" TEXT="Types">
<node CREATED="1393788201721" ID="ID_2500737" MODIFIED="1393819800180" TEXT="weighted keywords">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      In encrypted keyword search, a server obliviously searches over a collection of encrypted documents; however, these confidentiality guarantees (query privacy and data confidentiality) complicate most of the traditional relevancy scoring techniques found in the IR literature, like tf-idf, unless we relax query confidentiality a little.
    </p>
    <p>
      
    </p>
    <p>
      Spectral Bloom Filters
    </p>
    <p>
      
    </p>
    <p>
      May be used to keep track of word frequencies.
    </p>
    <p>
      
    </p>
    <p>
      ---
    </p>
    <p>
      
    </p>
    <p>
      Confidentiality-Preserving Rank-Ordered Search
    </p>
  </body>
</html>
</richcontent>
<node CREATED="1393794005513" ID="ID_344217566" MODIFIED="1393816341625" TEXT="Limitations">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      If using keyword weighting, like tf-idf, and we want to ensure query/data confidentiality, then we have certain limitations.
    </p>
    <p>
      
    </p>
    <p>
      If a word in one document maps to a different hash than the same word in another document, and we want to preserve query privacy, then we have a problem. Since only an authorized user knows how to perform this mapping (or if a proxy encryption scheme is used, only the user plus the server), then each document will need a different query which the server by itself cannot determine. You can have the user do this, but that eats up a lot of bandwidth and processing power.
    </p>
    <p>
      
    </p>
    <p>
      If we relax some of the query privacy restrictions, then we can do a lot better, e.g., take two separate queries. Query 1 has terms {hello, world} and query 2 has terms {hello, fujinoki}. If hello maps to the same hash value in both queries, this is a form of information leak: data access patterns are given, and as more data is accumulated about the access patterns, more confident statistical inferences may be made about what those patterns represent. Do note though that even though the queries leak this kind of information (it's still far less information leaked from a plaintext query), the documents themselves may not use that same mapping, e.g., given two documents which contain term1, said term may still map to a different hash/representations for each document, e.g., depending on the hash functions used for the index, and/or the server may need to hash the concatenation of the query term hashes and the document id for each document, etc.
    </p>
    <p>
      
    </p>
  </body>
</html>
</richcontent>
</node>
</node>
<node CREATED="1393790611658" ID="ID_1040751408" MODIFIED="1393814640650" TEXT="term proximity">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      Unexplorered in the context of encrypted searching.
    </p>
    <p>
      
    </p>
    <p>
      See: &quot;Efficient Text Proximity Search&quot; review for its benefits.
    </p>
    <p>
      
    </p>
    <p>
      Basic idea: given two documents, doc 1 = &quot;A B C&quot; and doc 2 = &quot;A D D ...&#160;&#160;D B C&quot;, doc 1 should be more relevant than doc 2 for the query &quot;A B&quot; even though they both contain the keywords with the same frequency. Put simply, how close together are the terms? The closer, all things else being equal, the better the match.
    </p>
    <p>
      
    </p>
    <p>
      Ideally, you use both weighted keywords (e.g., tf-idf) and term proximity measures. Any real system needs both to be able to effectively search through large collections.
    </p>
  </body>
</html>
</richcontent>
</node>
<node CREATED="1393790601304" ID="ID_1835473978" MODIFIED="1393813710826" TEXT="Semantic search">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      Concept Search: Semantics Enabled Information Retrieval
    </p>
    <p>
      
    </p>
    <p>
      Previous methods are variations of syntactic search -- bottom line, we're doing string matching (or, in many cases, we're hashing/transforming strings and comparing those hashes/transformations), combined with various ways to judge how important particular string matches are in the context of weighting the keyword (string) matches (tf-idf) and how important being &quot;close together&quot; (term proximity) is.
    </p>
    <p>
      
    </p>
    <p>
      Two major problems with syntactic search, however, has to do with: different words (or phrases) may be used to express similar meanings (depending on the context)--synonymy--and the same word (or phrase) may be used to express different meanings (depending on the context)--polysemy. Both of these problems harm the relevancy of results.
    </p>
    <p>
      
    </p>
    <p>
      Semantic search takes a different approach. What is the meaning of the text, and does the query &quot;match&quot; that meaning? This may involve a lot of techniques in syntactic searching, but in addition, uses methods like:
    </p>
    <ul>
      <li>
        natural language processing to perform word-sense disambiguation, part of speech tagging, and named entity recognition. When combining this with ontological / semantic knowledge and logical or probablistic inference, the IR system may begin to process queries in a way that resembles people's ability to understand text. For instance, if a user says, &quot;find me stuff that has a carnivore hunting prey&quot;, we may assume he is interested in &quot;a dog chasing cats&quot; or &quot;lions hunting antelopes&quot;. Using part of speech tagging, it can be determined that &quot;carnivore&quot; is the subject, &quot;hunting&quot; is the verb, and &quot;prey&quot; is the object. Using word-sense disambiguation, the word senses can be determined fairly accurately, e.g., &quot;carnivore&quot; maps to &quot;carnivore-1&quot; (word sense 1). Using an ontology (like Wordnet), it can be determined that &quot;carnivore-1&quot; is a concept which includes (more specific concepts) like &quot;dog-1&quot;, &quot;lion-2&quot;, etc. Then, we may assume we can expand the subject, &quot;carnivore-1&quot;, to {&quot;carnivore-1&quot;, &quot;dog-1&quot;, &quot;lion-2&quot;, ...}, the verb, &quot;hunting-3&quot;, to {&quot;hunting-3&quot;, &quot;chasing-1&quot;, &quot;preying-4&quot;, ...}, and the object &quot;prey-4&quot; to {&quot;prey-4&quot;, &quot;feline-2&quot;, &quot;antelope-1&quot;, &quot;cat-1&quot;, ...}. As you can see, this is not an easy problem, but this approach can already, in limited ways, be used to perform this kind of transformation from syntactic terms to conceptual terms.
      </li>
    </ul>
    <ul>
      <li>
        Statistical techniques to model semantically related terms

        <ul>
          <li>
            Latent semantic indexing is a prime example
          </li>
          <li>
            No formal logic, no formal ontology, no natural language processing, just pure statistical number crunching.
          </li>
        </ul>
      </li>
    </ul>
  </body>
</html>
</richcontent>
<node CREATED="1393809261257" ID="ID_1058852469" MODIFIED="1393813705097" TEXT="NLP"/>
<node CREATED="1393809269001" ID="ID_1843433090" MODIFIED="1393809276342" TEXT="statistical (LSI)"/>
</node>
</node>
<node CREATED="1393793804089" ID="ID_732365925" MODIFIED="1393811546650" TEXT="Likely use cases"/>
<node CREATED="1393976820649" ID="ID_865833835" MODIFIED="1393976823509" TEXT="query expansion">
<node CREATED="1393976828377" ID="ID_1541274063" MODIFIED="1393976832053" TEXT="drift"/>
</node>
</node>
</node>
<node CREATED="1393790708377" ID="ID_531028180" MODIFIED="1393790906246" POSITION="left" TEXT="Online versus offline (index)">
<node CREATED="1393790847417" ID="ID_784830976" MODIFIED="1393790911574" TEXT="Offline (index)">
<node CREATED="1393790831945" ID="ID_752896366" MODIFIED="1393883141387" TEXT="Inverted index">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      Adding Compression to Block Addressing Inverted Indexes
    </p>
  </body>
</html>
</richcontent>
<node CREATED="1393794468393" ID="ID_1177571382" MODIFIED="1393801719221" TEXT="Disadvantages">
<node CREATED="1393794474329" ID="ID_1862292236" MODIFIED="1393801742948" TEXT="What&apos;s being stored?">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      If words are being stored, then this means the attacker can see what the document has. No good, even if it's only storing them at the block level since this still leaks too much information.
    </p>
    <p>
      
    </p>
    <p>
      If encrypted or compressed or obfuscated versions of words are being stored, still vulnerable to frequency analysis.
    </p>
  </body>
</html>
</richcontent>
</node>
</node>
<node CREATED="1393794940777" ID="ID_1736567563" MODIFIED="1393794942918" TEXT="Advantages">
<node CREATED="1393794943848" ID="ID_986701910" MODIFIED="1393801891072" TEXT="Well-understood">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      Most popular choice. Very well understood and optimized.
    </p>
  </body>
</html>
</richcontent>
</node>
<node CREATED="1393794950585" ID="ID_1915121641" MODIFIED="1393801866226" TEXT="Fast">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      log n
    </p>
  </body>
</html>
</richcontent>
</node>
<node CREATED="1393794953832" ID="ID_1485254005" MODIFIED="1393801827332" TEXT="Space-efficient">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      Especially if block (or document) inverted index is used, e.g., each term has an association postings list at the document or block (block of document, or super-document) level, thus k words in a document can map to fewer positions than k.
    </p>
  </body>
</html>
</richcontent>
</node>
</node>
</node>
<node CREATED="1393790828665" ID="ID_1536090360" MODIFIED="1393813914337" STYLE="fork" TEXT="Bloom filter (signature)">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      Secure Indexes.
    </p>
    <p>
      Encrypted Keyword Search in a Distributed Storage System
    </p>
    <p>
      
    </p>
    <p>
      Leaks very little information. In a bloom filter, we have a lossy representation of the words, e.g., false positives, which means we can map many terms to the same thing. And mappings for different terms overlap, so one cannot know if a pattern is from a combination of n terms or a single term. It's really hard to reconstruct much information from this very lossy (but sufficient for membership queries) representation, which is why the bloom filter is such a string choice. Added to that, the bloom filter is also space efficient, since we trade space for false positives.
    </p>
    <p>
      
    </p>
    <p>
      It's not as space efficient as a min perfect hash (Network Applications of Bloom Filters: A Survey), which may also be used to emit false positives; the min perfect hash construction also allows for the evaluation of only two hash functions instead of k. However, bloom filters leak less information and are simpler to implement.
    </p>
  </body>
</html>
</richcontent>
<node CREATED="1393800655257" ID="ID_1708209409" MODIFIED="1393813912946" TEXT="Advantages">
<node CREATED="1393800660441" ID="ID_661933190" MODIFIED="1393813912946" TEXT="Space efficient"/>
<node CREATED="1393800664569" ID="ID_366157556" MODIFIED="1393813912946" TEXT="Fast"/>
</node>
<node CREATED="1393800679609" ID="ID_1792265711" MODIFIED="1393813912947" TEXT="Disadvantages">
<node CREATED="1393800682105" ID="ID_1404753384" MODIFIED="1393813912947" TEXT="False positives are possible (but controllable)"/>
</node>
<node CREATED="1393801335561" ID="ID_207530726" MODIFIED="1393813912947" TEXT="Types">
<node CREATED="1393801338041" ID="ID_1098615914" MODIFIED="1393813912947" TEXT="Spectral Bloom filters (multiset)">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      Can be used for word frequencies; no example of its usage in the encrypted searching literature.
    </p>
    <p>
      
    </p>
    <p>
      Word frequencies are important for relevancy measures. The frequencies can represent a range, instead of an exact value, to reduce information leakage and space complexity.
    </p>
  </body>
</html>
</richcontent>
</node>
<node CREATED="1393801383322" ID="ID_983756309" MODIFIED="1393813912947" TEXT="Classical (set)"/>
<node CREATED="1393813747129" ID="ID_622417922" MODIFIED="1393813912948" TEXT="Space-Code Bloom Filter (multiset)">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      Space-Code Bloom Filter for Efficient Per-Flow Traffic Measurement
    </p>
  </body>
</html>
</richcontent>
</node>
</node>
</node>
<node CREATED="1393813959722" ID="ID_790722618" MODIFIED="1393813978631" TEXT="Perfect Hash + Cryptographic Hash">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      Network Applications of Bloom Filters: A Survey
    </p>
  </body>
</html>
</richcontent>
</node>
</node>
<node CREATED="1393790856153" ID="ID_229526268" MODIFIED="1393810000981" TEXT="Online">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      Practical Techniques for Searches on Encrypted Data
    </p>
    <p>
      Public-key encryption with keyword search
    </p>
  </body>
</html>
</richcontent>
<node CREATED="1393794310201" ID="ID_1903196956" MODIFIED="1393798984732" TEXT="Advantages">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      Straight-forward to use.
    </p>
  </body>
</html>
</richcontent>
</node>
<node CREATED="1393794313145" ID="ID_792721456" MODIFIED="1393794315622" TEXT="Disadvantages"/>
</node>
</node>
<node CREATED="1393800037129" ID="ID_303692122" MODIFIED="1393816232004" POSITION="right" TEXT="Multi-user encrypted searching">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      See: Veri&#64257;able Symmetric Searchable Encryption for Multiple Groups of Users
    </p>
    <p>
      
    </p>
    <p>
      <font face="Calibri,sans-serif"><span style="font-family: Calibri,sans-serif">Most encrypted search implementations assume only one person would be able to perform the searches, or, if multiple people, then they all must share the same secret, and that secret would allow them to query the secure index. But what if we want to be able to be able to revoke the ability for a user to query the secure index? </span><span>&#160;</span><span style="font-family: Calibri,sans-serif">Take this example: <o p="#DEFAULT"></o></span></font>
    </p>
    <p style="text-indent: -.25in; margin-left: .5in" class="MsoNoSpacing">
      <font size="12.0pt" face="Symbol"><span>&#183;</span></font><font size="7.0pt" face="Times New Roman"><span style="font-variant: normal; font-weight: normal; font-style: normal; line-height: normal; font-family: Times New Roman; font-size: 7.0pt">&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160; </span></font><font size="12.0pt"><span style="font-size: 12.0pt">User 1 makes secure index for document using secret<o p="#DEFAULT"></o></span></font>
    </p>
    <p style="margin-left: .5in; text-indent: -.25in" class="MsoNoSpacing">
      <font size="12.0pt" face="Symbol"><span>&#183;</span></font><font size="7.0pt" face="Times New Roman"><span style="font-variant: normal; font-weight: normal; font-style: normal; font-family: Times New Roman; line-height: normal; font-size: 7.0pt">&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160; </span></font><font size="12.0pt"><span style="font-size: 12.0pt">User 2 is trusted to query secure index by sharing with him the secret<o p="#DEFAULT"></o></span></font>
    </p>
    <p style="margin-left: .5in; text-indent: -.25in" class="MsoNoSpacing">
      <font size="12.0pt" face="Symbol"><span>&#183;</span></font><font size="7.0pt" face="Times New Roman"><span style="font-variant: normal; font-weight: normal; font-style: normal; font-family: Times New Roman; line-height: normal; font-size: 7.0pt">&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160; </span></font><font size="12.0pt"><span style="font-size: 12.0pt">User 1 no longer wants user 2 to be able to query the secure index<o p="#DEFAULT"></o></span></font>
    </p>
    <p>
      <font face="Calibri,sans-serif"><span style="font-family: Calibri,sans-serif">How can this be accomplished? A solution is to simply use two secrets, and only give User 2 one of the secrets. Example:<o p="#DEFAULT"></o></span></font>
    </p>
    <p style="margin-left: .5in; text-indent: -.25in" class="MsoNoSpacing">
      <font size="12.0pt" face="Symbol"><span>&#183;</span></font><font size="7.0pt" face="Times New Roman"><span style="font-variant: normal; font-weight: normal; font-style: normal; font-family: Times New Roman; line-height: normal; font-size: 7.0pt">&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160; </span></font><font size="12.0pt"><span style="font-size: 12.0pt">User 1 makes secure index for document using secret<sub>1</sub>&#160;and secret<sub>2</sub><o p="#DEFAULT"></o></span></font>
    </p>
    <p style="margin-left: .5in; text-indent: -.25in" class="MsoNoSpacing">
      <font size="12.0pt" face="Symbol"><span>&#183;</span></font><font size="7.0pt" face="Times New Roman"><span style="font-variant: normal; font-weight: normal; font-style: normal; font-family: Times New Roman; line-height: normal; font-size: 7.0pt">&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160; </span></font><font size="12.0pt"><span style="font-size: 12.0pt">User 2 is trusted to query secure index, partly, by sharing with him secret<sub>1</sub><o p="#DEFAULT"></o></span></font>
    </p>
    <p style="margin-left: .5in; text-indent: -.25in" class="MsoNoSpacing">
      <font size="12.0pt" face="Symbol"><span>&#183;</span></font><font size="7.0pt" face="Times New Roman"><span style="font-variant: normal; font-weight: normal; font-style: normal; font-family: Times New Roman; line-height: normal; font-size: 7.0pt">&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160; </span></font><font size="12.0pt"><span style="font-size: 12.0pt">User 1 gives a server secret <sub>2</sub><o p="#DEFAULT"></o></span></font>
    </p>
    <p style="margin-left: .5in; text-indent: -.25in" class="MsoNoSpacing">
      <font size="12.0pt" face="Symbol"><span>&#183;</span></font><font size="7.0pt" face="Times New Roman"><span style="font-variant: normal; font-weight: normal; font-style: normal; font-family: Times New Roman; line-height: normal; font-size: 7.0pt">&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160; </span></font><font size="12.0pt"><span style="font-size: 12.0pt">User 2 may submit encrypted queries to server on secure index<o p="#DEFAULT"></o></span></font>
    </p>
    <p style="margin-left: .5in; text-indent: -.25in" class="MsoNoSpacing">
      <font size="12.0pt" face="Symbol"><span>&#183;</span></font><font size="7.0pt" face="Times New Roman"><span style="font-variant: normal; font-weight: normal; font-style: normal; font-family: Times New Roman; line-height: normal; font-size: 7.0pt">&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160; </span></font><font size="12.0pt"><span style="font-size: 12.0pt">Server cannot determine contents of encrypted query, nor contents of secure index, except which documents are ranked as relevant to the encrypted query<o p="#DEFAULT"></o></span></font>
    </p>
    <p style="margin-left: .5in; text-indent: -.25in" class="MsoNoSpacing">
      <font size="12.0pt" face="Symbol"><span>&#183;</span></font><font size="7.0pt" face="Times New Roman"><span style="font-variant: normal; font-weight: normal; font-style: normal; font-family: Times New Roman; line-height: normal; font-size: 7.0pt">&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160; </span></font><font size="12.0pt"><span style="font-size: 12.0pt">User 2 still needs the server because both secret<sub>1</sub>&#160;and secret<sub>2</sub>&#160; must be used to query secure index<o p="#DEFAULT"></o></span></font>
    </p>
    <p style="margin-left: .5in; text-indent: -.25in" class="MsoNoSpacing">
      <font size="12.0pt" face="Symbol"><span>&#183;</span></font><font size="7.0pt" face="Times New Roman"><span style="font-variant: normal; font-weight: normal; font-style: normal; font-family: Times New Roman; line-height: normal; font-size: 7.0pt">&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160; </span></font><font size="12.0pt"><span style="font-size: 12.0pt">User 1 no longer wants user 2 to be able to query the secure index. He informs server not to honor his query requests<o p="#DEFAULT"></o></span></font>
    </p>
    <p style="margin-left: .5in; text-indent: -.25in" class="MsoNoSpacing">
      <font size="12.0pt" face="Symbol"><span>&#183;</span></font><font size="7.0pt" face="Times New Roman"><span style="font-variant: normal; font-weight: normal; font-style: normal; font-family: Times New Roman; line-height: normal; font-size: 7.0pt">&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160; </span></font><font size="12.0pt"><span style="font-size: 12.0pt">Even if User 2 has a local copy of secure index, he cannot query it since he does not know secret<sub>2</sub>&#160;and he can&#8217;t ask the server to query it on his behalf since the server has been instructed to de-authorize him.<o p="#DEFAULT"></o></span></font>
    </p>
    <p class="MsoNoSpacing">
      <font size="12.0pt"><span style="font-size: 12.0pt"><o p="#DEFAULT">
      &#160;</o></span></font>
    </p>
    <p class="MsoNormal">
      In pseudo code, a term is hashed into the secure index like so (or something more elaborate):<o p="#DEFAULT"></o>
    </p>
    <p style="text-indent: .5in" class="MsoNormal">
      hash<sub>2</sub>(hash<sub>1</sub>(term<sub>plain text</sub>&#160;| secret<sub>1</sub>), secret<sub>2</sub>)<o p="#DEFAULT"></o>
    </p>
    <p class="MsoNormal">
      Thus, to check the secure index for term<sub>plain text</sub>, both secret<sub>1</sub>&#160; and secret<sub>2</sub>&#160;must be known.<o p="#DEFAULT"></o>
    </p>
    <p>
      <font face="Calibri,sans-serif"><span style="font-family: Calibri,sans-serif">This is the essential idea behind multi-user systems. Note that these models generally assume the server (who is trusted with one of the required secrets) and the partially trusted users do not collude. If server agrees to continue servicing requests of deauthorized users or if server gives the users secret<sub>2</sub>, then they will be able to continue querying the secure index.<o p="#DEFAULT"></o></span></font>
    </p>
  </body>
</html>
</richcontent>
<node CREATED="1393800053801" ID="ID_924856897" MODIFIED="1393800055430" TEXT="Use cases"/>
</node>
<node CREATED="1393904712265" ID="ID_214107445" MODIFIED="1393904724245" POSITION="left" TEXT=" document models">
<node CREATED="1393904724793" ID="ID_737923812" MODIFIED="1393904729044" TEXT="vector space">
<node CREATED="1393904755753" ID="ID_1831766864" MODIFIED="1393904756806" TEXT="problems"/>
</node>
<node CREATED="1393904729929" ID="ID_189922177" MODIFIED="1393904731333" TEXT="heuristic">
<node CREATED="1393904752696" ID="ID_453520183" MODIFIED="1393904754565" TEXT="problems"/>
</node>
<node CREATED="1393904732217" ID="ID_810506837" MODIFIED="1393904734389" TEXT="language model">
<node CREATED="1393904736681" ID="ID_1726398266" MODIFIED="1393904751205" TEXT="problems for encrypted search"/>
</node>
<node CREATED="1393904760473" ID="ID_410569505" MODIFIED="1393904768949" TEXT="concept search">
<node CREATED="1393904769536" ID="ID_593935060" MODIFIED="1393904770677" TEXT="problems"/>
</node>
</node>
</node>
</map>
