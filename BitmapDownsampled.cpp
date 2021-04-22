#include <string.h>
#include <utility>

#include "BitmapDownsampled.hpp"
#include "Debug.hpp"

#if defined __SSE4_1__ || defined __AVX2__ || defined _MSC_VER
#  ifdef _MSC_VER
#    include <intrin.h>
#  else
#    include <x86intrin.h>
#  endif
#endif


static const float SrgbToLinear[256] = {
    0.00000000f, 0.00030353f, 0.00060705f, 0.00091058f, 0.00121411f, 0.00151763f, 0.00182116f, 0.00212469f, 0.00242822f, 0.00273174f, 0.00303527f, 0.00334654f, 0.00367651f, 0.00402472f, 0.00439144f, 0.00477695f,
    0.00518152f, 0.00560539f, 0.00604883f, 0.00651209f, 0.00699541f, 0.00749903f, 0.00802319f, 0.00856813f, 0.00913406f, 0.00972122f, 0.01032982f, 0.01096010f, 0.01161225f, 0.01228649f, 0.01298303f, 0.01370208f,
    0.01444385f, 0.01520852f, 0.01599630f, 0.01680738f, 0.01764196f, 0.01850022f, 0.01938237f, 0.02028857f, 0.02121901f, 0.02217389f, 0.02315337f, 0.02415763f, 0.02518686f, 0.02624123f, 0.02732090f, 0.02842604f,
    0.02955684f, 0.03071345f, 0.03189604f, 0.03310477f, 0.03433981f, 0.03560132f, 0.03688945f, 0.03820437f, 0.03954624f, 0.04091520f, 0.04231142f, 0.04373503f, 0.04518621f, 0.04666509f, 0.04817183f, 0.04970657f,
    0.05126947f, 0.05286066f, 0.05448029f, 0.05612850f, 0.05780544f, 0.05951125f, 0.06124608f, 0.06301004f, 0.06480329f, 0.06662596f, 0.06847819f, 0.07036012f, 0.07227187f, 0.07421359f, 0.07618540f, 0.07818744f,
    0.08021984f, 0.08228272f, 0.08437622f, 0.08650047f, 0.08865561f, 0.09084174f, 0.09305899f, 0.09530749f, 0.09758737f, 0.09989875f, 0.10224175f, 0.10461651f, 0.10702312f, 0.10946173f, 0.11193245f, 0.11443539f,
    0.11697068f, 0.11953844f, 0.12213881f, 0.12477185f, 0.12743771f, 0.13013650f, 0.13286835f, 0.13563335f, 0.13843164f, 0.14126332f, 0.14412849f, 0.14702728f, 0.14995980f, 0.15292618f, 0.15592648f, 0.15896088f,
    0.16202942f, 0.16513222f, 0.16826941f, 0.17144111f, 0.17464741f, 0.17788842f, 0.18116425f, 0.18447499f, 0.18782078f, 0.19120169f, 0.19461782f, 0.19806932f, 0.20155625f, 0.20507872f, 0.20863685f, 0.21223074f,
    0.21586055f, 0.21952623f, 0.22322799f, 0.22696590f, 0.23074009f, 0.23455067f, 0.23839766f, 0.24228121f, 0.24620141f, 0.25015837f, 0.25415218f, 0.25818294f, 0.26225075f, 0.26635569f, 0.27049786f, 0.27467740f,
    0.27889434f, 0.28314883f, 0.28744090f, 0.29177073f, 0.29613835f, 0.30054384f, 0.30498737f, 0.30946898f, 0.31398878f, 0.31854683f, 0.32314327f, 0.32777816f, 0.33245158f, 0.33716366f, 0.34191447f, 0.34670410f,
    0.35153273f, 0.35640025f, 0.36130691f, 0.36625272f, 0.37123778f, 0.37626222f, 0.38132611f, 0.38642955f, 0.39157256f, 0.39675534f, 0.40197787f, 0.40724030f, 0.41254270f, 0.41788515f, 0.42326775f, 0.42869058f,
    0.43415371f, 0.43965724f, 0.44520128f, 0.45078585f, 0.45641109f, 0.46207705f, 0.46778387f, 0.47353154f, 0.47932023f, 0.48515001f, 0.49102089f, 0.49693304f, 0.50288659f, 0.50888145f, 0.51491779f, 0.52099568f,
    0.52711529f, 0.53327656f, 0.53947961f, 0.54572457f, 0.55201155f, 0.55834049f, 0.56471163f, 0.57112491f, 0.57758057f, 0.58407855f, 0.59061897f, 0.59720188f, 0.60382742f, 0.61049569f, 0.61720663f, 0.62396049f,
    0.63075721f, 0.63759696f, 0.64447975f, 0.65140569f, 0.65837491f, 0.66538733f, 0.67244321f, 0.67954254f, 0.68668550f, 0.69387192f, 0.70110208f, 0.70837593f, 0.71569365f, 0.72305530f, 0.73046088f, 0.73791057f,
    0.74540436f, 0.75294232f, 0.76052463f, 0.76815128f, 0.77582234f, 0.78353792f, 0.79129803f, 0.79910284f, 0.80695236f, 0.81484669f, 0.82278585f, 0.83076996f, 0.83879912f, 0.84687334f, 0.85499269f, 0.86315727f,
    0.87136722f, 0.87962234f, 0.88792318f, 0.89626944f, 0.90466136f, 0.91309869f, 0.92158204f, 0.93011087f, 0.93868589f, 0.94730657f, 0.95597351f, 0.96468627f, 0.97344548f, 0.98225057f, 0.99110222f, 1.00000000f,
};


static uint32_t LinearToSrgb( float v )
{
    if( v <= 0.0031308f )
    {
        return uint32_t( 255 * ( v * 12.92f ) );
    }
    else
    {
        return uint32_t( 255 * ( 1.055f * pow( v, 1.f / 2.4f ) - 0.055f ) );
    }
}


BitmapDownsampled::BitmapDownsampled( const Bitmap& bmp, unsigned int lines, bool linearize )
    : Bitmap( bmp, lines )
{
    m_size.x = std::max( 1, bmp.Size().x / 2 );
    m_size.y = std::max( 1, bmp.Size().y / 2 );

    int w = std::max( m_size.x, 4 );
    int h = std::max( m_size.y, 4 );

    DBGPRINT( "Subbitmap " << m_size.x << "x" << m_size.y );

    m_block = m_data = new uint32_t[w*h];

    if( m_size.x < w || m_size.y < h )
    {
        memset( m_data, 0, w*h*sizeof( uint32_t ) );
        m_linesLeft = h / 4;
        unsigned int lines = 0;
        for( int i=0; i<h/4; i++ )
        {
            for( int j=0; j<4; j++ )
            {
                lines++;
                if( lines > m_lines )
                {
                    lines = 0;
                    m_sema.unlock();
                }
            }
        }
        if( lines != 0 )
        {
            m_sema.unlock();
        }
    }
    else
    {
        m_linesLeft = h / 4;
        if( linearize )
        {
            m_load = std::async( std::launch::async, [this, &bmp, w, h]() mutable
            {
                auto ptr = m_data;
                auto src1 = bmp.Data();
                auto src2 = src1 + bmp.Size().x;
                unsigned int lines = 0;
                for( int i=0; i<h/4; i++ )
                {
                    for( int j=0; j<4; j++ )
                    {
                        int k = m_size.x;
                        while( k-- )
                        {
                            int r = ( ( *src1 & 0x000000FF ) + ( *(src1+1) & 0x000000FF ) + ( *src2 & 0x000000FF ) + ( *(src2+1) & 0x000000FF ) ) / 4;
                            int g = ( ( ( *src1 & 0x0000FF00 ) + ( *(src1+1) & 0x0000FF00 ) + ( *src2 & 0x0000FF00 ) + ( *(src2+1) & 0x0000FF00 ) ) / 4 ) & 0x0000FF00;
                            int b = ( ( ( *src1 & 0x00FF0000 ) + ( *(src1+1) & 0x00FF0000 ) + ( *src2 & 0x00FF0000 ) + ( *(src2+1) & 0x00FF0000 ) ) / 4 ) & 0x00FF0000;
                            int a = ( ( ( ( ( *src1 & 0xFF000000 ) >> 8 ) + ( ( *(src1+1) & 0xFF000000 ) >> 8 ) + ( ( *src2 & 0xFF000000 ) >> 8 ) + ( ( *(src2+1) & 0xFF000000 ) >> 8 ) ) / 4 ) & 0x00FF0000 ) << 8;
                            *ptr++ = r | g | b | a;
                            src1 += 2;
                            src2 += 2;
                        }
                        src1 += m_size.x * 2;
                        src2 += m_size.x * 2;
                    }
                    lines++;
                    if( lines >= m_lines )
                    {
                        lines = 0;
                        m_sema.unlock();
                    }
                }

                if( lines != 0 )
                {
                    m_sema.unlock();
                }
            } );
        }
        else
        {
            m_load = std::async( std::launch::async, [this, &bmp, w, h]() mutable
            {
                auto ptr = m_data;
                auto src1 = bmp.Data();
                auto src2 = src1 + bmp.Size().x;
                unsigned int lines = 0;
                for( int i=0; i<h/4; i++ )
                {
                    for( int j=0; j<4; j++ )
                    {
                        int k = m_size.x;
#ifdef __AVX2__
                        while( k > 2 )
                        {
                            k -= 2;

                            __m128i p0 = _mm_loadu_si128( (__m128i*)src1 );
                            __m128i p1 = _mm_loadu_si128( (__m128i*)src2 );
                            src1 += 4;
                            src2 += 4;

                            __m256i px0 = _mm256_cvtepu8_epi16( p0 );
                            __m256i px1 = _mm256_cvtepu8_epi16( p1 );

                            __m256i s0 = _mm256_add_epi16( px0, px1 );
                            __m256i s1 = _mm256_shuffle_epi32( s0, _MM_SHUFFLE( 1, 0, 3, 2 ) );
                            __m256i s2 = _mm256_add_epi16( s0, s1 );

                            __m256i r0 = _mm256_srli_epi16( s2, 2 );
                            __m256i r1 = _mm256_packus_epi16( r0, r0 );

                            *ptr++ = _mm_cvtsi128_si32( _mm256_castsi256_si128( r1 ) );
                            *ptr++ = _mm_cvtsi128_si32( _mm256_extracti128_si256( r1, 1 ) );
                        }
#endif
                        while( k-- )
                        {
#ifdef __SSE4_1__
                            uint64_t p0, p1;
                            memcpy( &p0, src1, 8 );
                            memcpy( &p1, src2, 8 );
                            src1 += 2;
                            src2 += 2;

                            __m128i px = _mm_set_epi64x( p0, p1 );
                            __m128i px0 = _mm_unpacklo_epi8( px, _mm_setzero_si128() );
                            __m128i px1 = _mm_unpackhi_epi8( px, _mm_setzero_si128() );

                            __m128i s0 = _mm_add_epi16( px0, px1 );
                            __m128i s1 = _mm_shuffle_epi32( s0, _MM_SHUFFLE( 1, 0, 3, 2 ) );
                            __m128i s2 = _mm_add_epi16( s0, s1 );

                            __m128i r0 = _mm_srli_epi16( s2, 2 );
                            __m128i r1 = _mm_packus_epi16( r0, r0 );

                            *ptr++ = _mm_cvtsi128_si32( r1 );
#else
                            int r = ( ( *src1 & 0x000000FF ) + ( *(src1+1) & 0x000000FF ) + ( *src2 & 0x000000FF ) + ( *(src2+1) & 0x000000FF ) ) / 4;
                            int g = ( ( ( *src1 & 0x0000FF00 ) + ( *(src1+1) & 0x0000FF00 ) + ( *src2 & 0x0000FF00 ) + ( *(src2+1) & 0x0000FF00 ) ) / 4 ) & 0x0000FF00;
                            int b = ( ( ( *src1 & 0x00FF0000 ) + ( *(src1+1) & 0x00FF0000 ) + ( *src2 & 0x00FF0000 ) + ( *(src2+1) & 0x00FF0000 ) ) / 4 ) & 0x00FF0000;
                            int a = ( ( ( ( ( *src1 & 0xFF000000 ) >> 8 ) + ( ( *(src1+1) & 0xFF000000 ) >> 8 ) + ( ( *src2 & 0xFF000000 ) >> 8 ) + ( ( *(src2+1) & 0xFF000000 ) >> 8 ) ) / 4 ) & 0x00FF0000 ) << 8;
                            *ptr++ = r | g | b | a;
                            src1 += 2;
                            src2 += 2;
#endif
                        }
                        src1 += m_size.x * 2;
                        src2 += m_size.x * 2;
                    }
                    lines++;
                    if( lines >= m_lines )
                    {
                        lines = 0;
                        m_sema.unlock();
                    }
                }

                if( lines != 0 )
                {
                    m_sema.unlock();
                }
            } );
        }
    }
}

BitmapDownsampled::~BitmapDownsampled()
{
}
