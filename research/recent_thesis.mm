<map version="1.0.1">
<!-- To view this file, download free mind mapping software FreeMind from http://freemind.sourceforge.net -->
<node CREATED="1391317348631" ID="ID_171023514" MODIFIED="1391335001279" TEXT="Thesis">
<edge STYLE="linear"/>
<node CREATED="1391352078453" ID="ID_1650343450" MODIFIED="1391442170668" POSITION="right" TEXT="searching">
<node CREATED="1391319696870" ID="ID_404259409" MODIFIED="1391701264136" TEXT="exact matching">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      Exact query matching is the simplest case.
    </p>
    <p>
      
    </p>
    <p>
      For an exact match on a n-query to happen, the bloomified message must support exact match n-grams.
    </p>
    <p>
      
    </p>
    <p>
      If the n-query does have a match in the bloomified message, and the bloomified message consists of N bloom filters at the largest accessible block granularity, then at most N bloom filter queries will be submitted by the query engine. This can even be done in parallel.
    </p>
  </body>
</html>
</richcontent>
<edge STYLE="linear"/>
</node>
<node COLOR="#338800" CREATED="1391440341717" ID="ID_886737196" MODIFIED="1391701262910" TEXT="approximate matching">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      If approximate search query matching is requested
    </p>
    <p>
      
    </p>
    <p>
      then it is mostly the task of the query engine to submit variations of the search query to find a minimal transformation of the query (using primitive operations), if one exists, which has a positive match. Bloom filters do not deal directly with these operations, and so the two components are independent modules.
    </p>
    <p>
      
    </p>
    <p>
      
    </p>
    <p>
      
    </p>
    <p>
      
    </p>
    <p>
      If approximate search query matching is requested, then it is mostly [1] the task of the query engine to submit variations of the search query to find a minimal transformation of the query (using primitive operations), if one exists, which has a positive match. Bloom filters do not deal directly with these operations, and so the two components are independent modules.
    </p>
    <p>
      
    </p>
    <p>
      Types of atomic query transformations are:
    </p>
    <ul>
      <li>
        permutations (when searching for an n-gram, there are n! permutations)
      </li>
      <li>
        character errors (e.g., mispellings)
      </li>
      <li>
        submatches
      </li>
    </ul>
    <p>
      [1] With the one exception in which bloom filters handle matching an ordered n-word search query with (n+k)-word search queries with k gaps. In this special case, locally sensitive search queries may be handled.
    </p>
  </body>
</html>
</richcontent>
<node CREATED="1391436844550" ID="ID_284391693" MODIFIED="1391698310813" TEXT="distance measures">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      When performing search queries, the query will either match the message exactly, or it will only match approximately.
    </p>
    <p>
      
    </p>
    <p>
      Given the nature of the bloom filter's design, the distance measures will have to cope with the fact that it only has very limited knowledge about the distance between the search query string and a hypothetical string represented in the bloom filter. This is handled by the distance metrics for locality, character, arrangement, and submatch.
    </p>
  </body>
</html></richcontent>
<node CREATED="1391370504822" ID="ID_42073970" MODIFIED="1391688434393" TEXT="locality metrics">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      <font face="Calibri Light">At the n-gram level, the locality measure is simple: 0 (unless we're using n-grams with gaps; a match upon that will have a locality measure equal to the number of gaps). </font>
    </p>
    <p>
      
    </p>
    <p>
      <font face="Calibri Light">However, most searches (except exact matches on n-grams) will probably be at the granularity of block sizes. </font>
    </p>
    <p>
      <font face="Calibri Light">&#160;</font>
    </p>
    <p>
      <font face="Calibri Light">Locality measure, at block level, is measured as follows. </font>
    </p>
    <p>
      
    </p>
    <p>
      <font face="Calibri Light">If a bloomified message only has, at most, bloom filters assigned </font>
    </p>
    <p>
      <font face="Calibri Light">to blocks with k-grams as members, then find all partition sets of </font>
    </p>
    <p>
      <font face="Calibri Light">an n-query where a partition's largest member is a k-query, e.g., </font>
    </p>
    <p>
      <font face="Calibri Light">if query is a 3-query, &quot;hello planet earth&quot;, and bloomified </font>
    </p>
    <p>
      <font face="Calibri Light">message only has up to 2-grams as members, then partitions are: </font>
    </p>
    <p>
      
    </p>
    <p>
      <font face="Calibri Light">{{hello, planet, earth}, {hello planet, earth}, {hello earth, planet}, </font>
    </p>
    <p>
      <font face="Calibri Light">{planet earth, hello}} </font>
    </p>
    <p>
      
    </p>
    <p>
      <font face="Calibri Light">4 partitions. Note that this number grows quite rapidly, which is </font>
    </p>
    <p>
      <font face="Calibri Light">why for large queries we will use approximate local (greedy) methods. </font>
    </p>
    <p>
      
    </p>
    <p>
      This is what we'll feed into our displacement measure.
    </p>
    <p>
      
    </p>
    <p>
      <font face="Calibri Light">Now, for a given partition, we're going to find out its locality measure </font>
    </p>
    <p>
      <font face="Calibri Light">against a&#160;bloomified message. If some of them have no </font>
    </p>
    <p>
      <font face="Calibri Light">match, we'll deal with that too (this will incur a submatch penalty, </font>
    </p>
    <p>
      <font face="Calibri Light">as defined by the submatching approximation error metric, </font>
    </p>
    <p>
      <font face="Calibri Light">although first we may elect to match the partition against character </font>
    </p>
    <p>
      <font face="Calibri Light">transformations of its terms, which incurs a character mismatch error </font>
    </p>
    <p>
      <font face="Calibri Light">as defined by the character transformation error metric, e.g., </font>
    </p>
    <p>
      <font face="Calibri Light">the &quot;edit-distance&quot; measure). </font>
    </p>
    <p>
      
    </p>
    <p>
      <font face="Calibri Light">Find all that do match, and then enumerate all of the possible block </font>
    </p>
    <p>
      <font face="Calibri Light">assignments for each element in the partition. Now, when N </font>
    </p>
    <p>
      <font face="Calibri Light">members (1-grams, 2-grams, ..., k-grams) fall into the same block, </font>
    </p>
    <p>
      <font face="Calibri Light">that has a cost of the block size. So, if a particular partition has M </font>
    </p>
    <p>
      <font face="Calibri Light">partitions and they fall into a total of K blocks, then just add up </font>
    </p>
    <p>
      <font face="Calibri Light">each of the K block sizes. </font>
    </p>
    <p>
      
    </p>
    <p>
      <font face="Calibri Light">displacement cost = {sum from i = 1 to K get_block_cost(i, x-grams in block[i]) * block[i].size}</font>
    </p>
    <p>
      
    </p>
    <p>
      <font face="Calibri Light">As you can see, we also apply a coefficient to each block that depends on a function of </font>
    </p>
    <p>
      <font face="Calibri Light">the average size of&#160;the x-grams matched. Specifically: </font>
    </p>
    <p>
      
    </p>
    <p>
      <font face="Calibri Light">get_block_cost(block, x-grams in block): weight </font>
    </p>
    <p>
      <font face="Calibri Light">{ </font>
    </p>
    <p>
      <font face="Calibri Light">&#160;&#160;&#160;&#160;sum = 0 </font>
    </p>
    <p>
      <font face="Calibri Light">&#160;&#160;&#160;&#160;for each x-gram in x-grams </font>
    </p>
    <p>
      <font face="Calibri Light">&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;sum += x-gram.size </font>
    </p>
    <p>
      <font face="Calibri Light">&#160;&#160;&#160;&#160;return x-grams.size / sum </font>
    </p>
    <p>
      <font face="Calibri Light">}</font>
    </p>
    <p>
      
    </p>
    <p>
      So, larger x-gram matches returns a smaller number, making it less costly.
    </p>
    <p>
      
    </p>
    <p>
      <font face="Calibri Light">Finally, we consider the dispersion. What is the dispersion of blocks used </font>
    </p>
    <p>
      <font face="Calibri Light">in a match? This is a simple calculation. The higher the dispersion, the </font>
    </p>
    <p>
      <font face="Calibri Light">worse the match. </font>
    </p>
    <p>
      
    </p>
    <p>
      Let B = set of blocks in the match. Then, central measure of
    </p>
    <p>
      blocks in B is defined as:
    </p>
    <p>
      
    </p>
    <p>
      central_measure(B):
    </p>
    <p>
      &#160;&#160;&#160;&#160;sum = 0
    </p>
    <p>
      &#160;&#160;&#160;&#160;for each b in B:
    </p>
    <p>
      &#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;sum += b.index
    </p>
    <p>
      &#160;&#160;&#160;&#160;return sum / size(B)
    </p>
    <p>
      
    </p>
    <p>
      dispersion_measure(B):
    </p>
    <p>
      &#160;&#160;&#160;&#160;sum = 0
    </p>
    <p>
      &#160;&#160;&#160;&#160;center = central_measure(B)
    </p>
    <p>
      &#160;&#160;&#160;&#160;for each b in B:
    </p>
    <p>
      &#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;sum += (b.index - center)^2
    </p>
    <p>
      &#160;&#160;&#160;&#160;return sqrt(sum / size(B))
    </p>
    <p>
      
    </p>
    <p>
      So, add this dispersion measure to the total:
    </p>
    <p>
      
    </p>
    <p>
      locality measure = a * displacement measure + b * dispersion measure, where a and b
    </p>
    <p>
      are weight coefficients.
    </p>
    <p>
      
    </p>
    <p>
      <font face="Calibri Light">Optionally, we can count block inversions and factor that </font>
    </p>
    <p>
      <font face="Calibri Light">into the final locality measure. </font>
    </p>
    <p>
      
    </p>
    <p>
      <font face="Calibri Light">Let's consider an example. Suppose we have a message with 3 blocks.</font>
    </p>
    <p>
      
    </p>
    <p>
      <font face="Consolas">+------------------------------+ 0 </font>
    </p>
    <p>
      <font face="Consolas">|&#160;&#160;&#160;&#160;&#160;&#160;&quot;hello&quot;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;| </font>
    </p>
    <p>
      <font face="Consolas">|&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&quot;planet&quot;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;| block 0 </font>
    </p>
    <p>
      <font face="Consolas">+------------------------------+ 20</font>
    </p>
    <p>
      <font face="Consolas">|&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&quot;earth&quot;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;| </font>
    </p>
    <p>
      <font face="Consolas">|&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;| block 1</font>
    </p>
    <p>
      <font face="Consolas">+------------------------------+ 40</font>
    </p>
    <p>
      <font face="Consolas">|&#160;&quot;hello&quot;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;| </font>
    </p>
    <p>
      <font face="Consolas">|&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&quot;hello planet&quot;&#160;&#160;| block 2</font>
    </p>
    <p>
      <font face="Consolas">+------------------------------+ 60</font>
    </p>
    <p>
      
    </p>
    <p>
      <font face="Calibri Light">If query = &quot;hello planet earth&quot;, there are 3 possible matches (note: word_n, n indicates block it is in): </font>
    </p>
    <p>
      
    </p>
    <p>
      hello_0 planet_0 earth_1
    </p>
    <p>
      &#160;&#160;&#160;&#160;locality measure = a*(20/1+20/1) + b*sqrt((0.5-0)^2+(0.5-1)^2)=40*a + 0.5*b
    </p>
    <p>
      hello_2 planet_0 earth_1
    </p>
    <p>
      &#160;&#160;&#160;&#160;locality measure = a*(20/1 + 20/1 + 20/1) + b*sqrt((1-0)^2+(1-1)^2+(1-2)^2) = 60*a + sqrt(2)*b
    </p>
    <p>
      &quot;hello planet&quot;_2 earth_1
    </p>
    <p>
      &#160;&#160;&#160;&#160;locality measure = a*(20 + 20/2) + b*sqrt((1.5-1)^2+(1.5-2)^2)= 30*a + 0.5*b
    </p>
    <p>
      
    </p>
    <p>
      Instead of block indexes in the dispersion calculation, it makes more sense to use actual offsets of blocks and block sizes, but this was easier to calculate.
    </p>
    <p>
      
    </p>
    <p>
      The results: the match with &quot;hello planet&quot; 2-gram was best match, match of 1-grams in 2 different blocks was second best, and match of 1-grams in 3 different blocks was the worst. In this case, any choice of weight coefficients 'a' and 'b' would rank them the same. But in general, the result depends on both the dispersion and the displacement measures. After all, it's probably better to match a bunch of 1-grams in 1 block, than fewer 2-grams in 2 blocks. But, these parameters can be tuned to the particular needs of the query.
    </p>
    <p>
      
    </p>
    <p>
      Note: the inclusion of being able to match k-grams, from k = 1 to k = n, and other factors blows up (a combinatorial explosion!) the search space for reasonably small n. We will use randomized approximate local methods nearly everywhere to quickly get good answers, hopefully even on arbitrarily large queries with 50+ terms in the query.
    </p>
  </body>
</html></richcontent>
</node>
<node CREATED="1391436967702" ID="ID_304346103" MODIFIED="1391693356953" TEXT="character metrics">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      character transformations (per word)
    </p>
    <p>
      
    </p>
    <p>
      given a search query and a number k, retrieve occurrences of words which can be
    </p>
    <p>
      transformed into the query with k errors. an error is an insertion, deletion, or replacement
    </p>
    <p>
      of a character.
    </p>
    <p>
      
    </p>
    <p>
      example
    </p>
    <p>
      -----------
    </p>
    <p>
      searching for occurrences of &quot;key&quot; with 0 edit errors:
    </p>
    <p>
      
    </p>
    <p>
      &#160;&#160;&#160;&#160;key
    </p>
    <p>
      
    </p>
    <p>
      searching for occurrences of &quot;key&quot; with 1 edit error:
    </p>
    <p>
      
    </p>
    <p>
      insert:
    </p>
    <p>
      &#160;&#160;&#160;&#160;key[a-z]
    </p>
    <p>
      &#160;&#160;&#160;&#160;ke[a-z]y
    </p>
    <p>
      &#160;&#160;&#160;&#160;k[a-z]ey
    </p>
    <p>
      &#160;&#160;&#160;&#160;[a-z]key
    </p>
    <p>
      
    </p>
    <p>
      replace:
    </p>
    <p>
      &#160;&#160;&#160;&#160;ke[a-z]
    </p>
    <p>
      &#160;&#160;&#160;&#160;k[a-z]y
    </p>
    <p>
      &#160;&#160;&#160;&#160;[a-z]ey
    </p>
    <p>
      
    </p>
    <p>
      delete:
    </p>
    <p>
      &#160;&#160;&#160;&#160;ke
    </p>
    <p>
      &#160;&#160;&#160;&#160;ky
    </p>
    <p>
      &#160;&#160;&#160;&#160;ey
    </p>
    <p>
      
    </p>
    <p>
      analysis:
    </p>
    <p>
      
    </p>
    <p>
      if S = character sequence size and Z is size
    </p>
    <p>
      of alphabet, then, number of variations is:
    </p>
    <p>
      
    </p>
    <p>
      &#160;&#160;&#160;&#160;N = Z(2S + 1)
    </p>
    <p>
      
    </p>
    <p>
      if S = 2 and Z = 2, we have 2(2*2+1) = 10 variations.
    </p>
    <p>
      
    </p>
    <p>
      let S = 3 and Z = 27, then N = 27(2*3+1) = 27*7 = 189.
    </p>
    <p>
      
    </p>
    <p>
      N ~ ZS, so the larger the alphabet or the larger the
    </p>
    <p>
      positions (size of word), the larger N is. For a fixed
    </p>
    <p>
      Z, N ~ S. This is only for 1-edit errors. For 2-edit
    </p>
    <p>
      errors, things grow much much quicker. It's simple
    </p>
    <p>
      enough to make a program traverse through all possible
    </p>
    <p>
      k-edit errors, but its slow and we have no way of telling
    </p>
    <p>
      how &quot;close&quot; an edit is to the bloomified message so we
    </p>
    <p>
      would have to enumerate and submit them individually.
    </p>
    <p>
      This is too much work for the system, so let's focus only
    </p>
    <p>
      on 1-edit errors.
    </p>
  </body>
</html></richcontent>
<node CREATED="1391693358949" ID="ID_1701346141" MODIFIED="1391693364850" TEXT="big data">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      We could instead use bloom filters that have
    </p>
    <p>
      as members edit errors of n-grams we insert into it.
    </p>
    <p>
      
    </p>
    <p>
      So, big data to the rescue? Let me explain:
    </p>
    <p>
      
    </p>
    <p>
      Why not learn, from very large word n-gram data sets,
    </p>
    <p>
      probable typos/misspellings/variations of it, and insert
    </p>
    <p>
      those into a bloom filter also? This would be
    </p>
    <p>
      done during the preprocessing stage. Now, we don't
    </p>
    <p>
      enumerate spelling variations because the bloom filter
    </p>
    <p>
      already has the most probable spelling variations for
    </p>
    <p>
      a given n-gram.
    </p>
    <p>
      
    </p>
    <p>
      If only a certain bloom filter set includes the spelling
    </p>
    <p>
      variations in this manner, then we could even include
    </p>
    <p>
      a character transformation penalty for any query
    </p>
    <p>
      that exactly matches the spelling variation. No need to
    </p>
    <p>
      lose that information about character approximation
    </p>
    <p>
      error.
    </p>
  </body>
</html></richcontent>
</node>
<node CREATED="1391320189286" ID="ID_1542433454" MODIFIED="1391698125491" TEXT="error types">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      When partial matches on words are desirable, e.g., misspellings of a word, then the query engine can handle this by automatically creating and submitting spelling variations of the word(s) in the search query.
    </p>
    <p>
      
    </p>
    <p>
      The degree of error is defined as the minimal number of changes needed to make the word test positive for membership in the bloom filter. Note that this feature can work in conjunction with the
    </p>
  </body>
</html></richcontent>
<edge STYLE="linear"/>
<node CREATED="1391320286070" ID="ID_384374902" MODIFIED="1391440404374" TEXT="number of errors">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      How many errors in a word are allowed?
    </p>
    <p>
      
    </p>
    <p>
      Note that the number of queries generated scales exponentially with the number of errors allowed and the size of the size of the word.
    </p>
  </body>
</html></richcontent>
<edge STYLE="linear"/>
</node>
<node CREATED="1391332018822" ID="ID_329922958" MODIFIED="1391335001278" TEXT="insertions">
<edge STYLE="linear"/>
</node>
<node CREATED="1391332033142" ID="ID_1226869521" MODIFIED="1391335001277" TEXT="deletions">
<edge STYLE="linear"/>
</node>
<node CREATED="1391332036438" ID="ID_21884219" MODIFIED="1391335001277" TEXT="replacements">
<edge STYLE="linear"/>
</node>
</node>
</node>
<node CREATED="1391436994806" ID="ID_1504896037" MODIFIED="1391696604211" TEXT="arrangement metrics">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      Arrangement measure is a measure of how distant a particular match in the bloomified
    </p>
    <p>
      message is to a particular partition of the n-query.
    </p>
    <p>
      
    </p>
    <p>
      That is, let's say we have a 3-query = a b c, which is a query consisting of 3
    </p>
    <p>
      words.
    </p>
    <p>
      
    </p>
    <p>
      The partitions are: {a b c}, {a b, c}, {a c, b}, {b c, a}, {a, b, c}.
    </p>
    <p>
      {a b c} represents a 3-gram query (does bloomfied message support 3-gram query
    </p>
    <p>
      matches exactly? Then we can try it for a perfect match.) {a b, c} represents
    </p>
    <p>
      a 2-gram query &quot;a b&quot; and a 1-gram query &quot;c&quot;. Does bloomified message support
    </p>
    <p>
      2-gram queries? Then we can try the 2-gram query for a perfect match, and
    </p>
    <p>
      indepdently the 1-gram query for a perfect match. {a, b, c} represents three
    </p>
    <p>
      1-gram queries. The bloomified message can always use this, since every one of
    </p>
    <p>
      them should have at least 1-gram granularity. Note, however, that this will
    </p>
    <p>
      result in a maximum arrangement penalty measure.
    </p>
    <p>
      
    </p>
    <p>
      Let's think about how to measure all of this.
    </p>
    <p>
      
    </p>
    <p>
      At the n-gram level, if an n-query is submitted to a bloomified message which
    </p>
    <p>
      supports exact matches up to n-grams, then the following arrangement distance
    </p>
    <p>
      measure is calculated: how many inversions are there? (That is, must we
    </p>
    <p>
      rearrange the n-query to get an exact match? Then count the inversions.) This is
    </p>
    <p>
      a simple calculation. The minimum number of inversions is 0 and the maximum
    </p>
    <p>
      number of inversions is 2n(n-1), which occurs when the query only exactly
    </p>
    <p>
      matches when it is reversed.
    </p>
    <p>
      
    </p>
    <p>
      Maybe this is not what we want, but it seems reasonable.
    </p>
    <p>
      
    </p>
    <p>
      If an exact match isn't found, then we try to find the best fit match. Note that
    </p>
    <p>
      I won't discuss, here, the fact that we can also do character distance measures
    </p>
    <p>
      on the individual words to try to find exact matches on those instead. This
    </p>
    <p>
      will impose a character distance measure penalty, however (which is usually
    </p>
    <p>
      relatively small compared to locality distance measure and arrangement
    </p>
    <p>
      distance measure).
    </p>
    <p>
      
    </p>
    <p>
      So, suppose an n-query is submitted against a bloomified message which has
    </p>
    <p>
      k-gram members up to size k = 3. Best case is if each of these k queries can be
    </p>
    <p>
      matched perfectly to the bloomfied message. If this is so, then we have not
    </p>
    <p>
      n members, but 3 members, and so we assume the very worst, that they, as 3-gram
    </p>
    <p>
      units, have maximum inversions with respect to the other 3-gram units. Instead
    </p>
    <p>
      of 2n(n-1), though, we have 2(n/3)(n/3-1) inversions. Comparing them:
    </p>
    <p>
      
    </p>
    <p>
      2n(n-1) / [2(n/3)(n/3-1)] = 9(n-1)/(n-3), which asymptotically converges
    </p>
    <p>
      to 9. So, best-case, the worst match is ~9 times greater than this one when
    </p>
    <p>
      assuming the 3-grams are maximally inverted with respect to each other. And,
    </p>
    <p>
      in general, perfectly matching k-grams is ~k times better.
    </p>
    <p>
      
    </p>
    <p>
      Let's talk about worst case. Worst case is when we only match n 1-grams, and
    </p>
    <p>
      assume they are maximally inverted with respect to each other. If so, we
    </p>
    <p>
      get back to 2n(n-1), like with the maximally inverted n-gram (but for which
    </p>
    <p>
      was otherwise a perfect match).
    </p>
    <p>
      
    </p>
    <p>
      Now, what about other combinations? Maybe we have a partition which consists
    </p>
    <p>
      of six 1-grams, two 2-grams, and one 3-grams. Worst case for this is
    </p>
    <p>
      n = 6+4+3, so 26(12) inversions. That's a big number. Best case is when the
    </p>
    <p>
      bloomified message has 3-gram members, and an exact match is found for the
    </p>
    <p>
      two 2-grams and one 3-grams. So, now our n = 6+2+1=9, so 18(8). Over a factor
    </p>
    <p>
      of 2 improvement.
    </p>
    <p>
      
    </p>
    <p>
      The actual locality measure, which is a separate module, will take care of
    </p>
    <p>
      measuring how far apart a particular partitioning is.
    </p>
    <p>
      
    </p>
  </body>
</html></richcontent>
<node CREATED="1391319735078" ID="ID_1971185540" MODIFIED="1391693974135" TEXT="n-grams with gaps">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      In locality metrics, we use a worst-case assumption on the locality measure.
    </p>
    <p>
      
    </p>
    <p>
      Say we search for &quot;hello planet&quot; and the message has a block that has
    </p>
    <p>
      in it the 3-gram &quot;hello beautiful planet&quot;. The best match for this is
    </p>
    <p>
      simply 1-gram matches on &quot;hello&quot; and &quot;planet&quot; in the same block,
    </p>
    <p>
      so worst-case assumption on distance measure is made.
    </p>
    <p>
      
    </p>
    <p>
      However, we could make 2-grams of the message, with 1 gap, members
    </p>
    <p>
      of the bloomified message. Then, the respective bloom filter for this block
    </p>
    <p>
      handling 1-gap members would have the member &quot;hello planet&quot;, which is
    </p>
    <p>
      an exact 2-gram match for the previous query. Now, we can still add a
    </p>
    <p>
      locality measurement error to this since we know the bloom filter dealing
    </p>
    <p>
      with this has 1-gap 2-gram members.
    </p>
    <p>
      
    </p>
    <p>
      We can do the same for 2-gaps, ..., n gaps.
    </p>
  </body>
</html></richcontent>
<edge STYLE="linear"/>
</node>
<node CREATED="1391693327446" ID="ID_573697669" MODIFIED="1391693513523" TEXT="big data">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      Using big data, we could look at all k-grams, and determine for any k-gram
    </p>
    <p>
      in our bloomified message that we wish to perfectly match, a probability distribution
    </p>
    <p>
      over the arrangements of those k terms in the k-gram. Then, we can make the most
    </p>
    <p>
      probable permutations of said k-gram members also (and assign a rearrangement
    </p>
    <p>
      error approximation to them, if desired). Then, we wouldn't have to do any
    </p>
    <p>
      permutations on the queries for approximate matching, which could significantly
    </p>
    <p>
      speed things up without blowing up the bloom filters necessarily.
    </p>
    <p>
      
    </p>
    <p>
      This is the same reasoning we applied to character metrics (edit-distance) big data section.
    </p>
  </body>
</html></richcontent>
</node>
<node CREATED="1391320037237" ID="ID_343298340" MODIFIED="1391692473162" TEXT="n-grams">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      Matching entire n-grams is dependent upon the bloom filter; the query engine cannot answer this question without support from the bloom filter.
    </p>
    <p>
      
    </p>
    <p>
      
    </p>
    <p>
      
    </p>
    <p>
      
    </p>
    <p>
      
    </p>
    <p>
      If a message is of size N, and query substrings (exact or permuted matches) up to size M are desired, then this operation has time complexity O(N*M*(M-1)* ... * 2) = O(NM!). Since, in general, M is small, e.g., M = 5 (allowing exact search queries up to 5 words long), the time complexity is O(N), which is linear with respect to N. That is, M can be treated as a constant.
    </p>
    <p>
      
    </p>
    <p>
      Combined with help from the query engine, this can optionally match any permutation of the M words without complicating the bloom filter itself. This is a form of approximate searching. Other techniques will allow far more sophisticated searching techniques without complicating the actual bloom filter. See: block-level granularity, substrings with gaps, character-level errors, and word-level errors.
    </p>
  </body>
</html></richcontent>
<edge STYLE="linear"/>
<icon BUILTIN="yes"/>
</node>
</node>
<node CREATED="1391697709510" ID="ID_350126812" MODIFIED="1391698105731" TEXT="submatch metrics">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      If only matching k out of n terms in the n-query, there should be a penalty applied--that is, it's more distant the fewer the terms. Worst-case match (but not non-zero) is only one of the terms out of the n terms matching, that is, only a 1-gram matches out of the n-gram query. Whether a submatch of k terms is less distant than a submatch of k+1 terms depends on the other distance measures.
    </p>
    <p>
      
    </p>
    <p>
      Do note there are 2^n submatches (0 being the submatch for which no match at all was found), so we cannot explore this exhaustively for large n-queries. Even the space for an k-query is quite large, since we would be exploring all of the partitions of this k-query (which is greater than 2^k). So, clearly, for many reasons, we will not be using exhaustive search methods, except for small queries.
    </p>
    <p>
      
    </p>
    <p>
      If the user wants a match that has all k terms in the k-query, then the query engine doesn't need to explore any k-query submatches.
    </p>
    <p>
      
    </p>
  </body>
</html></richcontent>
</node>
</node>
<node CREATED="1391319692582" ID="ID_1734695016" MODIFIED="1391693432243" TEXT="fuzzy matching">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      Fuzzy search queries provide a degree of membership value in the range [0, 1]. A degree of membership equal to 1 is an exact match. A degree of membership equal to 0 is when, given the approximation constraints, no match is found.
    </p>
    <p>
      
    </p>
    <p>
      Fuzzy search will use the results of an approximate match, which returns the approximation error. We will normalize this approximation error such that a maximum approximation error will have a value of 1 (we can do this if we define a metric space, see: well-defined metrics). Then, to return a degree of membership, we will simply return 1 - (normalized approximation error).
    </p>
  </body>
</html></richcontent>
<edge STYLE="linear"/>
</node>
<node CREATED="1391675171637" ID="ID_1429512451" MODIFIED="1391677080565" TEXT="finding solutions">
<node CREATED="1391677081654" ID="ID_972420421" MODIFIED="1391677215397" TEXT="optimal">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      Need to define well-defined metrics since for large queries we cannot explore entire state space.
    </p>
  </body>
</html></richcontent>
<node CREATED="1391676507740" FOLDED="true" ID="ID_750907629" MODIFIED="1391677664790" TEXT="well-defined metrics">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      These kind of well-defined metrics will help prune out a lot of the search space on any query matches. For instance, we only need to explore a certain submatch query size, and we only need to explore that submatch query in a certain order, from most optimal to least optimal.
    </p>
    <p>
      
    </p>
    <p>
      If we have the worst-possible match at that submatch size, that's still a potentially enormous state space to explore, so we can still explore it using local search methods which only guarantee finding a local maximum. Randomized restarts can get out of poor local maximums. This can only guarantee finding local maximums -- this is OK since the bloomified message itself is an approximation of the message! Let's find a reasonable solution quickly, without having to use too much memory or time.
    </p>
    <p>
      
    </p>
    <p>
      Note: the very best match for a k-query search happens when a k-gram granularity bloomified message has a perfect match to it, which has O(# of bloom filters) complexity. It's constant. Always try this one first.
    </p>
    <p>
      
    </p>
    <p>
      If we don't use the metric constraints, but we do use the distance metrics (e.g., we have the distance functions, but we don't have the constraints on those), then we can still use those metrics to provide a degree of membership estimate, according to those metrics. And, as stated previously, these metrics give us an upper bound on maximum number of approximation errors.
    </p>
  </body>
</html></richcontent>
<node CREATED="1391677014071" ID="ID_907054982" MODIFIED="1391677025901">
<richcontent TYPE="NODE"><html>
  <head>
    
  </head>
  <body>
    <img src="metric_equations.png" />
  </body>
</html></richcontent>
</node>
</node>
</node>
<node CREATED="1391677089142" ID="ID_308654232" MODIFIED="1391697707592" TEXT="greedy local searching">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      <font face="Consolas">basic template (gradient descent) approach of greedy searching </font>
    </p>
    <p>
      <font face="Consolas">--------------------------------------------------------------- </font>
    </p>
    <p>
      <font face="Consolas">input: </font>
    </p>
    <p>
      <font face="Consolas">&#160;&#160;&#160;&#160;partition: </font>
    </p>
    <p>
      <font face="Consolas">&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;a starting query partition (set of sets), e.g., for a k-query, which </font>
    </p>
    <p>
      <font face="Consolas">&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;is a k-gram, it could be a k partitioning with a 1-gram in each one: </font>
    </p>
    <p>
      <font face="Consolas">&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;{{ q1 }, { q2 }, ..., { qk }} </font>
    </p>
    <p>
      
    </p>
    <p>
      <font face="Consolas">&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;or it could be any other exhaustive partitioning, e.g.: </font>
    </p>
    <p>
      <font face="Consolas">&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;{{ q1, q3}, { q2, q4, q7}, ..., { qk, q6, q5, q10}} </font>
    </p>
    <p>
      
    </p>
    <p>
      <font face="Consolas">&#160;&#160;&#160;&#160;measures: </font>
    </p>
    <p>
      <font face="Consolas">&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;four measurement functions, each of which take in the partition: </font>
    </p>
    <p>
      
    </p>
    <p>
      <font face="Consolas">&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;(1) one for locality metrics: measure of how displaced the k </font>
    </p>
    <p>
      <font face="Consolas">&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;sets in the partition are. </font>
    </p>
    <p>
      
    </p>
    <p>
      <font face="Consolas">&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;(2) one for arrangement metrics: measure of how distant the query's </font>
    </p>
    <p>
      <font face="Consolas">&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;arrangement (sequence of 1-grams) is from the closest approximate </font>
    </p>
    <p>
      <font face="Consolas">&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;match in the bloom filter. </font>
    </p>
    <p>
      
    </p>
    <p>
      <font face="Consolas">&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;(3) one for character metrics: a measure of how edit distance, e.g., </font>
    </p>
    <p>
      <font face="Consolas">&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;how many edits had to be made to it to have it match? </font>
    </p>
    <p>
      
    </p>
    <p>
      <font face="Consolas">&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;(4) submatch penalty: if only matching k out of n, k &lt; n, terms, </font>
    </p>
    <p>
      <font face="Consolas">&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;apply a penalty.</font>
    </p>
    <p>
      
    </p>
    <p>
      <font face="Consolas">&#160;&#160;&#160;&#160;neighbors: </font>
    </p>
    <p>
      <font face="Consolas">&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;a function which returns some neighbors of the input. a lot of </font>
    </p>
    <p>
      <font face="Consolas">&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;pruning/approximation/randomization can be done here. </font>
    </p>
    <p>
      
    </p>
    <p>
      <font face="Consolas">&#160;&#160;&#160;&#160;steps: </font>
    </p>
    <p>
      <font face="Consolas">&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;number of steps to take before returning best found value</font>
    </p>
    <p>
      
    </p>
    <p>
      <font face="Consolas">--------------------------------------------------------------- </font>
    </p>
    <p>
      <font face="Consolas">greedy_find_minimum(partition, measures, neighbors, steps) -&gt; a solution </font>
    </p>
    <p>
      
    </p>
    <p>
      <font face="Consolas">&#160;&#160;&#160;&#160;&quot;Search for a query partition+transformation on query that minimizes </font>
    </p>
    <p>
      <font face="Consolas">&#160;&#160;&#160;&#160;measures(partition), considering neighbors of partition.&quot; </font>
    </p>
    <p>
      
    </p>
    <p>
      <font face="Consolas">&#160;&#160;&#160;&#160;distance = measures(partition) </font>
    </p>
    <p>
      <font face="Consolas">&#160;&#160;&#160;&#160;neighborhood = neighbors(partition) </font>
    </p>
    <p>
      <font face="Consolas">&#160;&#160;&#160;&#160;for i = 1 to steps </font>
    </p>
    <p>
      <font face="Consolas">&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;neighbor_partition = extract a neighbor from the neighborhood </font>
    </p>
    <p>
      <font face="Consolas">&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;distance2 = measures(partition) </font>
    </p>
    <p>
      <font face="Consolas">&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;if distance2 &lt;= distance: </font>
    </p>
    <p>
      <font face="Consolas">&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;partition, distance = neighbor_partition, distance2 </font>
    </p>
    <p>
      <font face="Consolas">&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;neighborhood = get_neighbors(partition) </font>
    </p>
    <p>
      <font face="Consolas">&#160;&#160;&#160;&#160;return {partition, distance} </font>
    </p>
    <p>
      <font face="Consolas">--------------------------------------------------------------- </font>
    </p>
    <p>
      
    </p>
    <p>
      <font face="Consolas">many things can be done to this template. for instance, to prevent expending </font>
    </p>
    <p>
      <font face="Consolas">a lot of work on already visited states, use a visited set (this could even </font>
    </p>
    <p>
      <font face="Consolas">be a bloom filter to keep space consumed by it low). </font>
    </p>
    <p>
      
    </p>
    <p>
      <font face="Consolas">we can also do random restarts to get out of local minima, and this is an </font>
    </p>
    <p>
      <font face="Consolas">embarrassingly&#160;parallel problem: every call to greedy_find_minimum is </font>
    </p>
    <p>
      <font face="Consolas">independent. </font>
    </p>
  </body>
</html></richcontent>
<node CREATED="1391677097382" ID="ID_281674886" MODIFIED="1391677099027" TEXT="randomization"/>
<node CREATED="1391697521014" ID="ID_172023799" MODIFIED="1391697522500" TEXT="approximate"/>
</node>
</node>
</node>
<node CREATED="1391334371000" ID="ID_354773284" MODIFIED="1391701264725" TEXT="preprocessing revisited">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      A form of approximate searching can be done in the preprocessing step by training a bloom filter on a transformation of the message. For instance, if we feed a bloom filter the output from preprocess(in) = soundex(extract_words(remove_stop_words(lower_case(in)))), then it will test membership on the basis of how similar a search query sounds to substrings in the original message.
    </p>
    <p>
      
    </p>
    <p>
      Besides the [potential] benefit of making searching more phonetic, Soundex has the added benefit that multiple words will map to the same exact sequence of characters, reducing the number of members further and thus simplifying any bloom filters that use it.
    </p>
  </body>
</html>
</richcontent>
<edge STYLE="linear"/>
<linktarget COLOR="#b0b0b0" DESTINATION="ID_354773284" ENDARROW="Default" ENDINCLINATION="-161;0;" ID="Arrow_ID_1475880717" SOURCE="ID_591674642" STARTARROW="None" STARTINCLINATION="-88;-96;"/>
<node CREATED="1391333118566" ID="ID_1800593811" MODIFIED="1391436541836" TEXT="phonetic algorithms">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      Phonetic algorithms include algorithms like Soundex. Soundex transforms words that approximately sound the same to the same hash.
    </p>
  </body>
</html></richcontent>
<edge STYLE="linear"/>
</node>
<node CREATED="1391364697205" ID="ID_1541323664" LINK="http://en.wikipedia.org/wiki/Locality-sensitive_hashing" MODIFIED="1391364749266" TEXT="locality-sensitive hashing"/>
</node>
</node>
<node CREATED="1391320132742" ID="ID_1169412406" MODIFIED="1391698334972" POSITION="right" TEXT="security">
<edge STYLE="linear"/>
<node CREATED="1391320137190" ID="ID_1609474370" MODIFIED="1391438591654" TEXT="specificity (of search)">
<edge STYLE="linear"/>
<arrowlink COLOR="#b0b0b0" DESTINATION="ID_420799865" ENDARROW="Default" ENDINCLINATION="38;-53;" ID="Arrow_ID_556498177" STARTARROW="None" STARTINCLINATION="157;53;"/>
<node CREATED="1391352211526" ID="ID_922622315" MODIFIED="1391698394151" TEXT="block granularity">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      The larger the block size, the larger the bounds on the minimum and maximum distance between any pairing of words that test positively as a member of the bloomified message.
    </p>
    <p>
      
    </p>
    <p>
      This makes it more difficult to determine the structure and content of a message since you can only locate&#160;n-gram&#160;proximity to some upper and lower bound defined by the block granularity.
    </p>
    <p>
      
    </p>
    <p>
      This upper and lower bound defined by the block granularity level can be used to derive a distance measure. See: locality distance measures.
    </p>
  </body>
</html></richcontent>
</node>
<node CREATED="1391352219157" ID="ID_109296732" MODIFIED="1391698563876" TEXT="n-gram granularity">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      Exact matches can only happen for n-gram word sequences, where n depends on the configuration of the bloom filter. If a bloomified message supports k-gram exact matches, but an n-query search is submitted, n &gt; k, then the system will do its best to find the best approximate match given the amount of information that can be derived from the n-gram granularity + block granularity.
    </p>
  </body>
</html></richcontent>
</node>
</node>
<node CREATED="1391440132629" ID="ID_1927328411" MODIFIED="1391440272897" TEXT="authorization">
<node CREATED="1391331233094" ID="ID_1766932165" MODIFIED="1391335001275" TEXT="asymmetric cyrptography">
<edge STYLE="linear"/>
</node>
</node>
</node>
<node CREATED="1391319771575" ID="ID_1175428538" LINK="http://en.wikipedia.org/wiki/Bloom_filter" MODIFIED="1391695403565" POSITION="left" TEXT="bloom filter">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      Binary classifiers answer the following query:
    </p>
    <ul>
      <li>
        Is a sample (e.g., example or feature vector) a member of the class or not?
      </li>
    </ul>
    <p>
      A bloom filter is a binary classifier. More precisely, a bloom filter answers the following query: is a particular sample a member of its set?
    </p>
    <p>
      
    </p>
    <p>
      If we make the n-grams (or some subset thereof) of a message the members of a bloom filter, then we can do search query operations on the bloom filter. And, more importantly, we can do this without revealing the large-scale structure or content (only at, say, the n-gram level) of the message. That is to say, we can represent a message (of textual data) as a bloom filter. This representation cannot be read, but it can be queried (within limitations), e.g., &quot;does this message contain the word 'alex'?&quot;.
    </p>
    <p>
      
    </p>
    <p>
      The most obvious n-gram to use is the 1-gram -- single words. By itself, this only allows us to ask the bloom filter if the message contains particular words. This may be useful, but when we combine the bloom filter with
    </p>
    <p>
      other data structures and algorithms, we can get yet a lot more mileage out of it, e.g., fuzzy approximate searching.
    </p>
    <p>
      
    </p>
    <p>
      One approach to expand the set of useful queries is to fragment a message into multiple blocks. Then, for each block, assign a separate bloom filter to it. Now, when we ask the system (of bloom filters) if it contains the query, &quot;hello world&quot;, it can not only determine whether the message contains both words, but it can also provide a measure of distance between these two words, e.g., if one word is in block i and another is in block j, by using meta-information about the blocks we can determine an upper and lower bound on how distant the two words are. We can also determine their order, e.g., if block i contains content that comes before block j, and block i contains &quot;hello&quot; and block j contains &quot;word&quot;, then the words are in order at the block level. (This may not be a very useful insight, however, since the blocks are, presumably, fairly large.)
    </p>
    <p>
      
    </p>
    <p>
      Another approach is to make not only 1-grams a member of the
    </p>
    <p>
      
    </p>
  </body>
</html></richcontent>
<edge STYLE="linear"/>
<node CREATED="1391319804486" ID="ID_357705572" MODIFIED="1391699226902" TEXT="training">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      Training is described as optimizing the parameters of the system to minimize the probability of a false positive.
    </p>
    <p>
      
    </p>
    <p>
      Note that we do not wish to reduce the false positive rate on improbable search queries; if it gets those wrong, it does not matter nearly as much. To reduce the probability of false positive, we must factor in the probability of false positives on PROBABLE queries (look at n-gram data sets and use the negative examples in those to train the parameters of the network). If improbable search queries result in false positives, like random sequences of characters, this is not such a problem. If it is important that improbable negative examples not have a false positive, this can be addressed to.
    </p>
    <p>
      
    </p>
    <p>
      Formally: p[false positive] =
    </p>
    <p>
      &#160;&#160;&#160;&#160;probability = 0
    </p>
    <p>
      &#160;&#160;&#160;&#160;for n-gram in negative examples:
    </p>
    <p>
      &#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;probability += p[false positive|n-gram]p[n-gram] }
    </p>
    <p>
      
    </p>
    <p>
      &#160;&#160;&#160;&#160;return probability
    </p>
    <p>
      
    </p>
    <p>
      
    </p>
    <p>
      We'll learn the following parameters:
    </p>
    <ul>
      <li>
        Number of hash functions for a bloom filter
      </li>
      <li>
        Types of hash functions

        <ul>
          <li>
            For a given hash function, vary parameters for it:
          </li>
        </ul>
      </li>
      <li>
        Number of bloom filters?
      </li>
      <li>
        Size of blocks?

        <ul>
          <li>
            Used for block-level granularity
          </li>
        </ul>
      </li>
    </ul>
  </body>
</html></richcontent>
<edge STYLE="linear"/>
<icon BUILTIN="yes"/>
<node CREATED="1391319807831" ID="ID_7531163" MODIFIED="1391335001277" TEXT="training set">
<edge STYLE="linear"/>
</node>
<node CREATED="1391319810534" ID="ID_1933458399" MODIFIED="1391335001277" TEXT="validation set">
<edge STYLE="linear"/>
</node>
<node CREATED="1391334534183" ID="ID_1630083720" MODIFIED="1391335001277" TEXT="">
<edge STYLE="linear"/>
<node CREATED="1391319950598" ID="ID_378661101" MODIFIED="1391335001276" TEXT="types">
<edge STYLE="linear"/>
<node CREATED="1391320700854" ID="ID_423357293" MODIFIED="1391699808877" TEXT="weighted n-grams">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <ul>
      <li>
        Use probablities of n-grams derivable from corpus of n-gram data sets findable on the internet

        <ul>
          <li>
            <p>
              We want to minimize false positives without complicating bloom filters
            </p>
            <ul>
              <li>
                minimize by training on probable negative examples (simple n-gram analysis)
              </li>
            </ul>
          </li>
          <li>
            Once an n-gram is included in training set, it will be classified correctly

            <ul>
              <li>
                Include all 1-grams for bloom filters handling 1-grams, but the n-grams for large n can be selectively included

                <ul>
                  <li>
                    Maybe this is wrong though? If an n-gram is rare but it is found in a message, maybe that gives said n-gram special importance
                  </li>
                </ul>
              </li>
            </ul>
          </li>
        </ul>
      </li>
    </ul>
  </body>
</html></richcontent>
<edge STYLE="linear"/>
<arrowlink COLOR="#b0b0b0" DESTINATION="ID_688959991" ENDARROW="Default" ENDINCLINATION="134;0;" ID="Arrow_ID_1708265006" STARTARROW="None" STARTINCLINATION="7;36;"/>
</node>
</node>
</node>
<node CREATED="1391319774566" ID="ID_1089751691" MODIFIED="1391699872796" TEXT="error types">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      True positive (TP): anything inserted into the bloom filter will subsequently always be tested as a true positive.
    </p>
    <p>
      False negative (FN): this is not possible with a bloom filter (Type II error).
    </p>
    <p>
      
    </p>
    <p>
      This means that the sensitivity, which is TP / (TP + FN), is 1. That is, the probability of a positive test, given that the message is a positive example, is certainly 1.
    </p>
    <p>
      
    </p>
    <p>
      Then, the only things we can improve are:
    </p>
    <p>
      
    </p>
    <p>
      True negative (TN): correctly tested as false
    </p>
    <p>
      False positive (FP): incorrectly tested as positive (Type I error)
    </p>
    <p>
      
    </p>
    <p>
      The metric we wish to improve is specificity:
    </p>
    <p>
      
    </p>
    <p>
      TN / (TN + FP) = TN / [total number of negative samples]. On the validation data sets, we want our bloom filter training (auto-tuning) to minimize FP (thus maximize TP).
    </p>
  </body>
</html></richcontent>
<edge STYLE="linear"/>
</node>
</node>
<node CREATED="1391359279862" ID="ID_1481768858" MODIFIED="1391438613086" TEXT="statistical properties">
<icon BUILTIN="yes"/>
</node>
<node CREATED="1391327617510" ID="ID_688959991" LINK="http://en.wikipedia.org/wiki/N-gram" MODIFIED="1391699936774" TEXT="n-grams">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      P[n-gram | message comes from some uniform distribution] can be modelled as a categorical distribution. What is the probability of seeing a particular n-gram?
    </p>
    <p>
      
    </p>
    <p>
      ---
    </p>
    <p>
      How are you generating the n-gram? If you generate it from a particular message repository about medical science, then the n-gram models P[n-gram | message comes from a medical science distribution]. Or, hopefully, P[n-gram | message is about medical science]. So, conversely:
    </p>
    <p>
      
    </p>
    <ul>
      <li>
        P[message is about medical science | n-gram] = P[n-gram | message is about medical science] * P[message is about medical science] / P[n-gram].
      </li>
      <li>
        So, if we have a bunch of n-grams, assume i.d.d.:<br /><br />P[message is about medical science | n-gram evidence] =<br /><br />product from i = 1 to k { P[n-gram[i] | message is about medical science] * P[message is about medical science] / P[n-gram[i]] }<br /><br />
      </li>
      <li>
        We can model what a document is about this way. Is this a new line of research? It's not new in the sense that this approach is common, but has it been done with bloom filters?
      </li>
    </ul>
    <p>
      
    </p>
    <p>
      Another question n-grams can model: given that the previous n-1 words were, what is the probability that word n is &lt;word&gt;? Note that this is saying that model makes word n independent of any word k &lt; n - 1. This is a Markov model in which any particular state depends only on the n - 1 previous states. P[x[i] given x[i-1], x[i-2], ..., x[i-n+1]]
    </p>
    <p>
      
    </p>
    <p>
      Another question n-grams can model: given that, starting from i, words j, j = {i, i+1, ..., i+k-1, i+k+1, ..., i+n}, what is the probability word k is &lt;word&gt;? This can be more simply modelled if we ignore words i+k+1 to i+n and only focus on words i to i+k-1, which would mean we would only use a k-grams model, k &lt; n.
    </p>
  </body>
</html></richcontent>
<edge STYLE="linear"/>
<linktarget COLOR="#b0b0b0" DESTINATION="ID_688959991" ENDARROW="Default" ENDINCLINATION="134;0;" ID="Arrow_ID_1708265006" SOURCE="ID_423357293" STARTARROW="None" STARTINCLINATION="7;36;"/>
</node>
</node>
<node CREATED="1391320390406" ID="ID_591674642" MODIFIED="1391655905964" POSITION="left" TEXT="preprocessing">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      How do we preprocess the message before providing it as input to a bloom filter?
    </p>
    <p>
      
    </p>
    <p>
      Algorithmically, the preprocessing is handled by a preprocessing function. By default, it is defined as: preprocess(in) = extract_words(remove_stop_words(lower_case(in)))
    </p>
    <p>
      
    </p>
    <p>
      But it can be anything, including the identity function: preprocess(in) = in.
    </p>
    <p>
      
    </p>
    <p>
      Note that each bloom filter can, in theory, go through a different preprocessing function. See: preprocessing revisited
    </p>
  </body>
</html></richcontent>
<edge STYLE="linear"/>
<arrowlink COLOR="#b0b0b0" DESTINATION="ID_354773284" ENDARROW="Default" ENDINCLINATION="-161;0;" ID="Arrow_ID_1475880717" STARTARROW="None" STARTINCLINATION="-88;-96;"/>
<node CREATED="1391320436806" ID="ID_395908789" MODIFIED="1391335001275" TEXT="stop words">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      Stop words are words like &quot;the&quot; and &quot;and&quot;; they carry no meaningful information so they are removed.
    </p>
  </body>
</html></richcontent>
<edge STYLE="linear"/>
</node>
<node CREATED="1391320501126" ID="ID_583758642" MODIFIED="1391336529563" TEXT="case-insensitivity">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      We do not care about case information. &quot;Alex&quot; -&gt; &quot;alex&quot;
    </p>
    <p>
      
    </p>
  </body>
</html></richcontent>
<edge STYLE="linear"/>
</node>
<node CREATED="1391320514374" ID="ID_27259176" MODIFIED="1391336364319" TEXT="word extraction">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      Only include words (minus stop words), e.g., a message like &quot;Hello, Alex. Who is $##$bill?&quot; will map to &quot;hello alex who is bill&quot;.
    </p>
  </body>
</html></richcontent>
<edge STYLE="linear"/>
<node CREATED="1391336039558" ID="ID_1547931707" MODIFIED="1391336354317" TEXT="definition of a word">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      Operational definition: only alphanumeric character sequences qualify as words.
    </p>
    <p>
      
    </p>
    <p>
      Examples: Hello, world! -&gt; Hello world
    </p>
  </body>
</html></richcontent>
</node>
</node>
<node CREATED="1391359410198" ID="ID_585627311" MODIFIED="1391655950997" TEXT="pre-hashing">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      Language is very structured. Seeing a character sequence like &quot;ljojsdf ajh4 aa&quot; or &quot;laugh pencil ocean you&quot; is unlikely. So, given that language samples character sequences from an extremely non-uniform distribution (n-grams may be used to model the actual distribution), why not do a many-to-one transformation, in which these improbable sequences of characters or words will map to probable sequences. In general, nothing will be lost by this sort of collision because an insufficient number of such improbable queries are being submitted.
    </p>
    <p>
      
    </p>
    <p>
      So, let the space of tokens be infinite, and map this infinite space to a finite space, e.g., every word maps to n bytes. By the pigeonhole principle, this necessarily means that for every index in the hash, an infinite number of character sequences will map to it. However, if these collisions are between one meaningful token and an infinite number of unmeaningful tokens, then nothing is lost; we way may reasonably assume that if the position (n bytes) that a meaningful token maps to is true, then the meaningful token that is true. It is also, however, possible that 2 or more meaningful tokens can map to the same position. The key is to choose hash functions for which this is unlikely to occur. If meaningful collisions can be made sufficiently improbable, then it should not be a point of much concern. After all, bloom filters already allow for false positives.
    </p>
    <p>
      
    </p>
    <p>
      Statistically, the reduced sample space has more variance and uniformity than the original sample space. It is this property that makes the transformation useful since. The next step is to insert the tokens in the transformed message into the bloom filter. Given the statistical properties of the tokens, it is hypothesized that it may be easier to generate effective bloom filters of smaller size and fewer and simpler hash functions for a given false positive rate.
    </p>
    <p>
      
    </p>
    <p>
      Another advantage of pre-hashing: even if the hash function is very expensive, it&#160;must only be performed for each word (non-stop word) once. Subsequently, every other hash function can re-use this hash function's output and hash that value instead. Moreover, this hash can be used for every subsequent
    </p>
    <p>
      
    </p>
    <p>
      See: compressed hashes
    </p>
  </body>
</html></richcontent>
</node>
</node>
<node CREATED="1391369194245" ID="ID_1406147124" MODIFIED="1391694242036" POSITION="left" TEXT="blocks">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      Fragment message into N blocks. Assign a bloom filter to each block (auto-tune it for its particular block).
    </p>
    <ul>
      <li>
        Do not have to create &quot;physical&quot; blocks -- they can be logical instead: start of block, size of block.

        <ul>
          <li>
            In this way, we do not need to duplicate data if using multiple block hierarchies.

            <ul>
              <li>
                How does the logical (where there is only one copy of the message) affect reconstruction costs? (See: mitigating reconstruction costs)
              </li>
              <li>
                It improves it. Why?

                <ul>
                  <li>
                    In a &quot;physical&quot; block approach, where each block is a separate copy, multi-level block hierarchies have major drawbacks. On a given level in the hierarchy, only the modified block's bloom filter needs to be reconstructed, but one or more blocks in different hierarchies will each need to be reconstructed separately. For instance, a very coarse &quot;whole message&quot; block (are these words anywhere in the document?) will need to be reconstructed if any lower-level block is modified.
                  </li>
                  <li>
                    In a logical block, we have no duplicates. At worst, the whole message needs to be reconstructed. Worst-case for &quot;physical&quot; block approach: every block of every level needs to be reconstructed, which if we have N blocks per level and M levels, that means we have to reconstruct the entire file M times.

                    <ul>
                      <li>
                        To allow for most modifications to a block to not require any changes, instead of padding to determine block boundaries, we can use delimeters (e.g., 5 nulls).

                        <ul>
                          <li>
                            Unfortunately, this prevents or complicates random access to the blocks.

                            <ul>
                              <li>
                                The block boundaries can be determined once for every modification that changes them and subsequently reused
                              </li>
                            </ul>
                          </li>
                        </ul>
                      </li>
                    </ul>
                  </li>
                </ul>
              </li>
              <li>
                Unfortunately, different hierarchical levels in the logical block approach may not use different encryptions. Only the &quot;base level&quot; blocks may use encryption; any one accessing the content in a higher-level block will need to get the encryption keys for each of the lower-level blocks that is a subset of the higher-level block.

                <ul>
                  <li>
                    &quot;Physical&quot; blocks, due to their redundancy, have no problems separately encrypting each block independently.
                  </li>
                </ul>
              </li>
            </ul>
          </li>
        </ul>
      </li>
    </ul>
  </body>
</html></richcontent>
<node CREATED="1391369517430" ID="ID_873389932" MODIFIED="1391369519315" TEXT="logical"/>
<node CREATED="1391369521318" ID="ID_334760572" MODIFIED="1391369523763" TEXT="physical"/>
<node CREATED="1391371418646" ID="ID_1086042714" MODIFIED="1391694172313" TEXT="overlapping blocks">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      Suppose we have a message that reads, &quot;hello word this is alex towell is anyone out there&quot;.
    </p>
    <p>
      
    </p>
    <p>
      If we fragment this message into two blocks like so:
    </p>
    <p>
      
    </p>
    <p>
      block 1: &quot;hello world this is alex&quot;
    </p>
    <p>
      block 2: &quot;towell is anyone out there&quot;
    </p>
    <p>
      
    </p>
    <p>
      Suppose we assign a 2-grams bloom filter to each block. Then, bloom filter 1 has members {&quot;hello world&quot;, &quot;world this&quot;, &quot;this is&quot;, &quot;is alex&quot;} and
    </p>
    <p>
      bloom filter 2 has members {&quot;towell is&quot;, &quot;is anyone&quot;, &quot;anyone out&quot;, &quot;out there&quot;}.
    </p>
    <p>
      
    </p>
    <p>
      Now, suppose a search query for &quot;alex towell&quot; is submitted. Clearly, this exact 2-gram is not a member of either bloom filter.
    </p>
    <p>
      If we had additional bloom filters for each block's 1-grams, and approximate matching was desired, then the most that can be
    </p>
    <p>
      said about the relationship between &quot;alex towell&quot; and the message is that &quot;alex&quot; is in block 1 and &quot;towell&quot; is in block 2, which
    </p>
    <p>
      means these two words are a minimum distance of 0 words apart and a maximum distance of 8 words apart.
    </p>
    <p>
      
    </p>
    <p>
      If it is desirable to allow exact matches up to 2-grams, then this is not an ideal situation. The&#160;reason we could not get an exact match has to do with the fact that the n-grams were generated separately for each block. The 2-gram, &quot;alex towell&quot;, is an extract 2-gram in the message, but not in the 2-grams in each block independently.
    </p>
    <p>
      
    </p>
    <p>
      A solution to this problem will only increase the complexity slightly: use overlapping blocks.
    </p>
    <p>
      
    </p>
    <p>
      For instance, the previous message is instead fragmented like so:
    </p>
    <p>
      
    </p>
    <p>
      block 1: &quot;hello world this is alex towell&quot;
    </p>
    <p>
      block 2: &quot;towell is anyone out there&quot;
    </p>
    <p>
      
    </p>
    <p>
      Now, bloom filter 1 has members {&quot;hello world&quot;, &quot;world this&quot;, &quot;this is&quot;, &quot;is alex&quot;, &quot;alex towell&quot;} and
    </p>
    <p>
      bloom filter 2 has members &quot;towell is&quot;, &quot;is anyone&quot;, &quot;anyone out&quot;, &quot;out there&quot;.
    </p>
    <p>
      
    </p>
    <p>
      A solution, then, is if we are fragmenting a message into blocks, and we want to exactly match against
    </p>
    <p>
      n-gram search queries, then for adjacent blocks i and j, i &lt; j, make the last (n-1) words common to both.
    </p>
    <p>
      
    </p>
    <p>
      Note that for overlapping blocks, the offset + size of block i will be greater than the offset of block j, i &lt; j.
    </p>
  </body>
</html></richcontent>
</node>
</node>
<node CREATED="1391357682758" ID="ID_1659903829" MODIFIED="1391695405316" POSITION="left" TEXT="future work">
<node CREATED="1391319780166" ID="ID_1462078629" MODIFIED="1391357935731" TEXT="other classifiers">
<edge STYLE="linear"/>
<node CREATED="1391357966646" ID="ID_201824112" MODIFIED="1391357968468" TEXT="naive bayes"/>
<node CREATED="1391357960821" ID="ID_1966577521" MODIFIED="1391357965011" TEXT="neural network"/>
</node>
<node CREATED="1391363963494" ID="ID_1903984511" MODIFIED="1391486369566" TEXT="mitigating reconstruction costs">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      If a message is modified, its bloom filters may need to be reconstructed at some point.
    </p>
    <ul>
      <li>
        A degree of staleness in the bloom filters may be acceptable though.
      </li>
    </ul>
    <p>
      However, if we use block-granularity bloom filters, then in theory only the affected blocks need to be reconstructed.
    </p>
  </body>
</html></richcontent>
<node CREATED="1391364175638" ID="ID_557766673" MODIFIED="1391491184006" TEXT="block granularity"/>
<node CREATED="1391364200758" ID="ID_1710187502" MODIFIED="1391412902792" TEXT="detecting non-meaningful edits">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      If a user modifies the structure of the message by:
    </p>
    <ul>
      <li>
        Adding/removing stop words
      </li>
      <li>
        Adding/removing whitespace / non-alphanumeric characters
      </li>
      <li>
        Changing the casing of any characters (lower case or upper case)
      </li>
    </ul>
    <p>
      Then the modification is not meaningful from the perspective of the bloom filter. That is, such changes do not cause the bloom filter to change if you reconstruct them using as input the modified message. Thus, these&#160;modifications do not necessarily call for a reconstruction task. However, do note that if we are using (logical or physical) blocks, then such changes may call for updating block boundary meta-information.
    </p>
    <p>
      
    </p>
    <p>
      A relevant question, then, is will it cost more to reconstruct the bloom filter unconditionally or detect such unmeaningful changes to avoid the reconstruction cost? That is, how frequently are edits of this kind? For instance, it is my suspicion that whitespace is frequently the only target of a modification; however, I have no hard data to back this up.
    </p>
  </body>
</html></richcontent>
<node CREATED="1391364218646" ID="ID_404014620" MODIFIED="1391364220227" TEXT="stop words"/>
<node CREATED="1391364221766" ID="ID_699459843" MODIFIED="1391412280677" TEXT="non-alphanumeric characters"/>
<node CREATED="1391412641462" ID="ID_1899164681" MODIFIED="1391412652499" TEXT="changing character cases"/>
</node>
</node>
<node CREATED="1391442048118" ID="ID_1431383340" MODIFIED="1391442056051" TEXT="bloom filter networks">
<node CREATED="1391442058054" ID="ID_78973668" MODIFIED="1391442284346" TEXT="multinomial classification">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      Construct a bloom filter, or a set of bloom filters, for each type of desired classification. This is, essentially, a single-layer bloom filter network.
    </p>
  </body>
</html></richcontent>
</node>
<node CREATED="1391442122406" ID="ID_1760452891" MODIFIED="1391442651453" TEXT="multi-layer hash (or bloom) networks">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      Instead of a single layer of hash functions, as in bloom filters, use multiple layers.
    </p>
    <p>
      
    </p>
    <p>
      When a bloom filter in a lower-layer processes input, it will output a boolean value indicating membership.
    </p>
    <p>
      
    </p>
  </body>
</html></richcontent>
</node>
</node>
<node CREATED="1391442873462" ID="ID_376296316" MODIFIED="1391490924238" TEXT="complex queries">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      A complex search query allows for logical operations like conjunctions, disjunctions, negations on the terms of the search query. See: BNF.
    </p>
    <p>
      
    </p>
    <p>
      So, search queires like &quot;(NOT(term_1) OR term_2) AND term_3&quot; can be performed. This complex query must then be decomposed, by the query engine, into multiple atomic queries. For instance, the prior complex query can be decomposed into the following sequence of atomic queries, along with instructions that correctly execute the query:
    </p>
    <p>
      
    </p>
    <p>
      &#160;&#160;&#160;&#160;let q1 = result of searching for term_1
    </p>
    <p>
      &#160;&#160;&#160;&#160;let q2 = result of searching for term_2
    </p>
    <p>
      &#160;&#160;&#160;&#160;let q3 = result of searching for term_3
    </p>
    <p>
      
    </p>
    <p>
      &#160;&#160;&#160;&#160;// q1, q2, and q3 are boolean values
    </p>
    <p>
      &#160;&#160;&#160;&#160;let q4 = NOT(q1)
    </p>
    <p>
      &#160;&#160;&#160;&#160;let q5 = OR(q2, q4)
    </p>
    <p>
      &#160;&#160;&#160;&#160;let q6 = AND(q3, q5)
    </p>
    <p>
      &#160;&#160;&#160;&#160;return q6
    </p>
  </body>
</html></richcontent>
<node CREATED="1391364888054" ID="ID_1568989104" MODIFIED="1391491454156" TEXT="propositional fuzzy logic">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      When discussing approximate matching of complex queries, we mentioned that to work with
    </p>
    <p>
      &#160;all we need to do to work with approximate errors as opposed to boolean values
    </p>
    <p>
      
    </p>
    <p>
      The approximation error can be transformed into a measure of its degree of membership. That is, transform the approximation
    </p>
    <p>
      error into a value in the range [0, 1], 0 indicating.
    </p>
    <p>
      
    </p>
    <p>
      Since the results of search queries can be fuzzy (degree of membership) values between 0 and 1, we can apply fuzzy operators to the returned values
    </p>
    <p>
      instead. (NOTE: The above algorithm's short-circuit approach is not correct if dealing with fuzzy values. It only requires slight tweaking, e.g., let something be true
    </p>
    <p>
      if its degree of membership is above K, then &quot;if (not(q1))&quot; maps to &quot;if (not(q1)) &gt;= K then return not(q1)&quot;.)
    </p>
    <p>
      
    </p>
    <p>
      If we need to work with fuzzy values in a more sophisticated way, see: defuzzification.
    </p>
    <p>
      
    </p>
    <p>
      Fuzzy logic equivalents for NOT, OR, and AND:
    </p>
    <ul>
      <li>
        NOT(q) = 1 - q
      </li>
      <li>
        AND(q1, q2) = MIN(q1, q2)
      </li>
      <li>
        OR(q1, q2) = MAX(q1, q2)
      </li>
    </ul>
    <p>
      We can also apply hedges to fuzzy degree of membership values. For example, let very(q) = q^2. Then, very(or(q1, q2)) will only be more than K, K in
    </p>
    <p>
      range [0, 1], if:
    </p>
    <ul>
      <li>
        very(or(q1, q2)) = max(q1^2, q2^2) &gt; K, then to be true it must be the case that q1 &gt; sqrt(K) or q2 &gt; sqrt(K).
      </li>
    </ul>
    <p>
      NOTE: very(q) is quadratic, where q is in range [0, 1]. This means that the rate of change of very(q) is 2q as opposed
    </p>
    <p>
      to q, which is 1. Also, very(q) reaches a maximum at q = 1, and a minium at q = 0, just like with the linear function,
    </p>
    <p>
      identity(q) = q. However, very(q) is lower everywhere in the range (0, 1) than identity(q), and it is the furthest apart from
    </p>
    <p>
      identify(q) at q = 1/2. If our defuzzification test for whether a fuzzy membership value is true is, say, K = 1/2, then
    </p>
    <p>
      q is true whenever q &gt; 1/2, but very(q) is true only when q &gt; sqrt(1/2) = 1 / sqrt(2) ~ 0.71. This makes since because
    </p>
    <p>
      we are asking if &quot;q is very true&quot; not just &quot;q is true&quot;.
    </p>
  </body>
</html></richcontent>
<node CREATED="1391366043398" ID="ID_1492019967" MODIFIED="1391366052595" TEXT="operators"/>
<node CREATED="1391366054742" ID="ID_578669072" MODIFIED="1391366056643" TEXT="hedges"/>
</node>
<node CREATED="1391490535510" ID="ID_909625774" MODIFIED="1391490651393" TEXT="BNF grammar">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      A full grammar (expressed in BNF notation) for propositional compound queries would look something like:
    </p>
    <p>
      
    </p>
    <p>
      &lt;query&gt;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;-&gt; &lt;atomic query&gt; | &lt;complex query&gt;
    </p>
    <p>
      &#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&lt;atomic query&gt; &lt;binary op&gt; &lt;query&gt;
    </p>
    <p>
      &#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&lt;complex query&gt; &lt;binary op&gt; &lt;query&gt;
    </p>
    <p>
      &lt;atomic query&gt;&#160;&#160;&#160;&#160;-&gt; &quot;&lt;string&gt;&quot;
    </p>
    <p>
      &lt;complex query&gt; -&gt; &lt;unary op&gt;(&lt;query&gt;)
    </p>
    <p>
      &#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;-&gt; (&lt;query&gt; &lt;binary op&gt; &lt;query&gt;)
    </p>
    <p>
      &lt;string&gt;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;-&gt; &lt;alphanumeric&gt; &lt;alphanumeric&gt;
    </p>
    <p>
      &lt;unary op&gt;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;-&gt; not | ...
    </p>
    <p>
      &lt;binary op&gt;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;-&gt; and | or | ...
    </p>
    <p>
      &lt;alphanumeric&gt;&#160;&#160;&#160;-&gt; [a-z0-9]+
    </p>
  </body>
</html></richcontent>
</node>
<node CREATED="1391490903589" ID="ID_190198113" MODIFIED="1391491334095" TEXT="approximate matching">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      If approximate searching is requested, then the return value from a search query will not be a boolean indicating whether a match was found. Rather, the result for each atomic query returns a measure of its minimum approximation error. (The approximation error can be something like the weighted linear combination of all the error types. See: searching::approximate matching.)
    </p>
    <p>
      
    </p>
    <p>
      To convert this approximation error into a boolean, we only need to define a boolean function that takes as input the approximation error and returns as output a crisp boolean. For instance:
    </p>
    <ul>
      <li>
        TRUE(q) = return q &lt; K
      </li>
    </ul>
    <p>
      That is, the approximation error, q, must be less than K for it to be true. For example, suppose the user submits the search query, q = &quot;(NOT(term_1) OR term_2) AND term_3&quot;. An implementation of this request might look like:
    </p>
    <p>
      
    </p>
    <p>
      &#160;&#160;&#160;&#160;let q1 = approximate error of searching for term_1
    </p>
    <p>
      &#160;&#160;&#160;&#160;let q2 = approximate error of searching for term_2
    </p>
    <p>
      &#160;&#160;&#160;&#160;let q3 = approximate error of searching for term_3
    </p>
    <p>
      
    </p>
    <p>
      &#160;&#160;&#160;&#160;// q1, q2, and q3 are approximation errors for each approximate search result
    </p>
    <p>
      &#160;&#160;&#160;&#160;let q4 = NOT(TRUE(q1))
    </p>
    <p>
      &#160;&#160;&#160;&#160;let q5 = OR(TRUE(q2), TRUE(q4))
    </p>
    <p>
      &#160;&#160;&#160;&#160;let q6 = AND(TRUE(q3), TRUE(q5))
    </p>
    <p>
      &#160;&#160;&#160;&#160;return TRUE(q6)
    </p>
  </body>
</html></richcontent>
</node>
</node>
<node CREATED="1391429347751" ID="ID_1150498106" MODIFIED="1391493061166" TEXT="message similarity">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      If two messages use the same exact configuration (for each bloom filter, they must have the same set of hash functions and size), then a measure of their similarity is how many bits are different?
    </p>
    <p>
      
    </p>
    <p>
      If someone were to wish to combine these two messages, they could perform a bit-wise OR on each position in the bloom filter.
    </p>
  </body>
</html></richcontent>
<node CREATED="1391429730839" ID="ID_420799865" MODIFIED="1391438591655" TEXT="attack vector">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      If users have access to the configuration of a bloom filter (e.g., they have sufficient security clearance to access one for the purpose of making queries against a message), then they could run, essentially, a key attack.
    </p>
    <p>
      
    </p>
    <p>
      A bloom filter maps every message to a boolean vector (which represents the member set). If they know the configuration of the bloom filter that performs the mapping for the message of interest, then they could try various candidate messages, input it into the bloom filter, and compare their respective boolean vector outputs . If they are equivalent, they are likely the same message. (Do note, however, that since we are mapping an arbitrarily large space (of messages) to a finite space (of boolean vectors of dimension N), an arbitrarily large number of messages will map to the same boolean vector.) If they are only slightly different, this may at least tell them something about their similarities.
    </p>
    <p>
      
    </p>
    <p>
      The vast proportion of messages in the sample space are so unlikely to be messages of interest (unless they are interested in a uniform distribution of characters) that a key attack of this nature may be possible. It is still too vast of a vast space to explore, however, without some insight into the actual exact contents of the message. If the attacker already has reason to believe it may be exactly such a message, any further damage caused by the attacker verifying the equivalency of the messages will probably be relatively minimal.
    </p>
  </body>
</html></richcontent>
<linktarget COLOR="#b0b0b0" DESTINATION="ID_420799865" ENDARROW="Default" ENDINCLINATION="38;-53;" ID="Arrow_ID_556498177" SOURCE="ID_1609474370" STARTARROW="None" STARTINCLINATION="157;53;"/>
</node>
</node>
<node CREATED="1391441163382" ID="ID_901081392" MODIFIED="1391700024560" TEXT="topic searching">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      P[n-gram | message comes from some uniform distribution] can be modelled as a categorical distribution. What is the probability of seeing a particular n-gram?
    </p>
    <p>
      
    </p>
    <p>
      How are you generating the n-gram? If you generate it from a particular message repository about medical science, then the n-gram models P[n-gram | message comes from a medical science distribution]. Or, hopefully, P[n-gram | message is about medical science]. So, conversely:
    </p>
    <ul>
      <li>
        P[message is about medical science | n-gram] = P[n-gram | message is about medical science] * P[message is about medical science] / P[n-gram].
      </li>
      <li>
        So, if we have a bunch of n-grams, assume i.d.d.:<br /><br />P[message is about medical science | n-gram evidence] = product from i = 1 to k { P[n-gram[i] | message is about medical science] * P[message is about medical science] / P[n-gram[i]] }<br /><br />
      </li>
      <li>
        We can model what a document is about this way. Is this a new line of research? It's not new in the sense that this approach is common, but has it been done with bloom filters?
      </li>
    </ul>
  </body>
</html></richcontent>
</node>
</node>
<node CREATED="1391492398710" ID="ID_159782864" MODIFIED="1391692754126" POSITION="right" TEXT="nomenclature">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      term: a legal word that can be searched for, e.g., alphanumeric sequence of characters
    </p>
    <p>
      k-query: a search query with k terms, where each term is separated by whitespace
    </p>
    <p>
      bloomified message: a message (document) which is being represented by a set of bloom filters.
    </p>
    <p>
      bloom filter: ...
    </p>
    <p>
      approximate matching: ...
    </p>
    <p>
      fuzzy matching: ...
    </p>
    <p>
      query engine: ...
    </p>
  </body>
</html></richcontent>
<icon BUILTIN="yes"/>
</node>
<node COLOR="#000000" CREATED="1391439357126" ID="ID_1988035970" MODIFIED="1391492683376" POSITION="right" TEXT="query engine">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      The query engine, at least initially, will only support atomic queries. Atomic queries consist of a single search term, e.g., &quot;hello world&quot;. They do not have disjunctions, conjunctions, or negations, e.g., &quot;NOT('hello world') OR 'alex'&quot;. See: future work::complex queries.
    </p>
  </body>
</html></richcontent>
<icon BUILTIN="yes"/>
</node>
<node CREATED="1391692059750" ID="ID_1411369405" MODIFIED="1391692064019" POSITION="right" TEXT="side notes">
<node CREATED="1391495243622" ID="ID_480856609" MODIFIED="1391692199511" TEXT="n-gram match sanity check">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      If a large n-gram has a match in a bloomified message, all 1-grams of the n-gram must also match. I think this will be automatically taken care of, but just wanted to comment.
    </p>
  </body>
</html></richcontent>
<icon BUILTIN="yes"/>
</node>
</node>
</node>
</map>
